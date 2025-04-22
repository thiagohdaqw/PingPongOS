CFLAGS = -ggdb -Wall -lm
BASE_FILES = src/ppos.c src/queue.c src/pqueue.c
BASE_INCLUDES = -Isrc/

testQueue: $(BASE_FILES) tests/queueTest.c
	$(CC) $(CFLAGS) -o build/queueTest $(BASE_INCLUDES) $(BASE_FILES) tests/queueTest.c

testPqueue: $(BASE_FILES) tests/pqueueTest.c
	$(CC) $(CFLAGS) -o build/pqueueTest $(BASE_INCLUDES) $(BASE_FILES) tests/pqueueTest.c


testContext: tests/contextTest.c
	$(CC) $(CFLAGS) -o build/contextTest $(BASE_INCLUDES) $(BASE_FILES) tests/contextTest.c

testTask: tests/taskTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -DDEBUG -o build/taskTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskTest.c

testTaskSwitch: tests/taskSwitchTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskSwitchTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskSwitchTest.c

testTaskStart: tests/taskStartTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskStartTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskStartTest.c

testTaskDispatcher: tests/taskDispatcherTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskDispatcherTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskDispatcherTest.c

testTaskPriority: tests/taskPriorityTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskPriorityTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskPriorityTest.c

testDispatcherTimer: tests/dispatcherTimerTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testDispatcherTimer $(BASE_INCLUDES) $(BASE_FILES) tests/dispatcherTimerTest.c

testDispatcherStressTimer: tests/dispatcherTimerStressTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testDispatcherStressTimer $(BASE_INCLUDES) $(BASE_FILES) tests/dispatcherTimerStressTest.c

testTime: tests/timeTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testTime $(BASE_INCLUDES) $(BASE_FILES) tests/timeTest.c
