// SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

/*
 * Do NOT include this file directly - include the QCoroTask header instead!
 */

#pragma once

#include "../qcorotask.h"

namespace QCoro::detail
{

inline TaskPromiseBase::TaskPromiseBase() {
    refCoroutine();
}

inline std::suspend_never TaskPromiseBase::initial_suspend() const noexcept {
    return {};
}

inline auto TaskPromiseBase::final_suspend() const noexcept {
    return TaskFinalSuspend{mAwaitingCoroutines};
}

template<typename T, typename Awaiter>
inline auto TaskPromiseBase::await_transform(T &&value) {
    return Awaiter{std::forward<T>(value)};
}

template<typename T>
inline QCoro::Task<T> &&TaskPromiseBase::await_transform(QCoro::Task<T> &&task) {
    return std::forward<QCoro::Task<T>>(task);
}

template<typename T>
inline QCoro::Task<T> &TaskPromiseBase::await_transform(QCoro::Task<T> &task) {
    return task;
}

template<Awaitable T>
inline T && TaskPromiseBase::await_transform(T &&awaitable) {
    return std::forward<T>(awaitable);
}

template<Awaitable T>
inline T &TaskPromiseBase::await_transform(T &awaitable) {
    return awaitable;
}

inline QCoro::CoroutineFeatures &TaskPromiseBase::await_transform(detail::ThisCoroPromise &&) {
    return mFeatures;
}

inline void TaskPromiseBase::addAwaitingCoroutine(std::coroutine_handle<> awaitingCoroutine) {
    mAwaitingCoroutines.push_back(awaitingCoroutine);
}

inline bool TaskPromiseBase::hasAwaitingCoroutine() const {
    return !mAwaitingCoroutines.empty();
}

inline void TaskPromiseBase::derefCoroutine() {
    --mRefCount;
    if (mRefCount == 0) {
        destroyCoroutine();
    }
}

inline void TaskPromiseBase::refCoroutine() {
    ++mRefCount;
}

inline CoroutineFeatures &TaskPromiseBase::features() {
    return mFeatures;
}

inline void TaskPromiseBase::destroyCoroutine() {
    mRefCount = 0;

    auto handle = std::coroutine_handle<TaskPromiseBase>::from_promise(*this);
    handle.destroy();
}

} // namespace QCoro::detail