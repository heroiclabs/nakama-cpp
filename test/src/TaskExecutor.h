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

#include <functional>
#include <list>
#include <utility>

namespace Nakama {
namespace Test {

class TaskExecutor
{
public:
    using TaskFunc = std::function<void()>;

    static TaskExecutor& instance();

    void addTask(TaskFunc&& task);
    void addTask(const TaskFunc& task);
    void currentTaskCompleted();
    void tick();

private:
    TaskExecutor() {}
    ~TaskExecutor() {}

private:
    struct TaskData
    {
        TaskData(TaskFunc&& func) : func(func) {}

        TaskFunc func;
        bool inProgress = false;
    };
    std::list<TaskData> _tasks;
};

} // namespace Test
} // namespace Nakama
