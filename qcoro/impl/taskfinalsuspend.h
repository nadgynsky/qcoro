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

inline TaskFinalSuspend::TaskFinalSuspend(const std::vector<std::coroutine_handle<>> &awaitingCoroutines)
    : mAwaitingCoroutines(awaitingCoroutines) {}

inline bool TaskFinalSuspend::await_ready() const noexcept {
    return false;
}

template<typename Promise>
inline void TaskFinalSuspend::await_suspend(std::coroutine_handle<Promise> finishedCoroutine) noexcept {
    auto &finishedPromise = finishedCoroutine.promise();
    for (auto &awaiter : mAwaitingCoroutines) {
        auto handle = std::coroutine_handle<TaskPromiseBase>::from_address(awaiter.address());
        auto &awaitingPromise = handle.promise();
        const auto &features = awaitingPromise.features();
        if (const auto &guardedThis = features.guardedThis(); guardedThis.has_value() && guardedThis->isNull()) {
            // We have a QPointer but it's null which means that the observed QObject has been destroyed.
            awaitingPromise.derefCoroutine();
        } else {
            awaiter.resume();
        }
    }
    mAwaitingCoroutines.clear();

    // The handle will be destroyed here only if the associated Task has already been destroyed
    finishedPromise.derefCoroutine();
}

constexpr void TaskFinalSuspend::await_resume() const noexcept {}

} // namespace QCoro::detail