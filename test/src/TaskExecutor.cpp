/*
 * Copyright 2019 The Nakama Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TaskExecutor.h"
#include <assert.h>

namespace Nakama {
namespace Test {

TaskExecutor& TaskExecutor::instance()
{
    static TaskExecutor theInstance;
    return theInstance;
}

void TaskExecutor::addTask(TaskFunc&& task)
{
    _tasks.emplace_back(TaskData(std::move(task)));
}

void TaskExecutor::addTask(const TaskFunc& task)
{
    _tasks.emplace_back(TaskData(TaskFunc(task)));
}

void TaskExecutor::currentTaskCompleted()
{
    assert(!_tasks.empty());
    assert(_tasks.front().inProgress);
    _tasks.pop_front();
}

void TaskExecutor::tick()
{
    if (!_tasks.empty())
    {
        TaskData& taskData = _tasks.front();

        if (!taskData.inProgress)
        {
            taskData.inProgress = true;
            // execute the task
            taskData.func();
        }
    }
}

} // namespace Test
} // namespace Nakama
