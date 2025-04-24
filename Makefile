CFLAGS = -ggdb -Wall
LINK_FLAGS = -lm
BASE_FILES = src/ppos.c src/ppos_ipc.c src/ppos_mqueue.c src/queue.c src/pqueue.c
BASE_INCLUDES = -Isrc/

testQueue: $(BASE_FILES) tests/queueTest.c
	$(CC) $(CFLAGS) -o build/queueTest $(BASE_INCLUDES) $(BASE_FILES) tests/queueTest.c $(LINK_FLAGS)

testPqueue: $(BASE_FILES) tests/pqueueTest.c
	$(CC) $(CFLAGS) -o build/pqueueTest $(BASE_INCLUDES) $(BASE_FILES) tests/pqueueTest.c $(LINK_FLAGS)


testContext: tests/contextTest.c
	$(CC) $(CFLAGS) -o build/contextTest $(BASE_INCLUDES) $(BASE_FILES) tests/contextTest.c $(LINK_FLAGS)

testTask: tests/taskTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -DDEBUG -o build/taskTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskTest.c $(LINK_FLAGS)

testTaskSwitch: tests/taskSwitchTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskSwitchTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskSwitchTest.c $(LINK_FLAGS)

testTaskStart: tests/taskStartTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskStartTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskStartTest.c $(LINK_FLAGS)

testTaskDispatcher: tests/taskDispatcherTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskDispatcherTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskDispatcherTest.c $(LINK_FLAGS)

testTaskPriority: tests/taskPriorityTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/taskPriorityTest $(BASE_INCLUDES) $(BASE_FILES) tests/taskPriorityTest.c $(LINK_FLAGS)

testDispatcherTimer: tests/dispatcherTimerTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testDispatcherTimer $(BASE_INCLUDES) $(BASE_FILES) tests/dispatcherTimerTest.c $(LINK_FLAGS)

testDispatcherStressTimer: tests/dispatcherTimerStressTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testDispatcherStressTimer $(BASE_INCLUDES) $(BASE_FILES) tests/dispatcherTimerStressTest.c $(LINK_FLAGS)

testTime: tests/timeTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testTime $(BASE_INCLUDES) $(BASE_FILES) tests/timeTest.c $(LINK_FLAGS)

testTaskSuspend: tests/taskSuspendTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testTaskSuspend $(BASE_INCLUDES) $(BASE_FILES) tests/taskSuspendTest.c $(LINK_FLAGS)

testTaskSleep: tests/taskSleepTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testTaskSleep $(BASE_INCLUDES) $(BASE_FILES) tests/taskSleepTest.c $(LINK_FLAGS)

testSemaphore: tests/semaphoreTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testSemaphore $(BASE_INCLUDES) $(BASE_FILES) tests/semaphoreTest.c $(LINK_FLAGS)

testSemaphoreStress: tests/semaphoreStressTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testSemaphoreStress $(BASE_INCLUDES) $(BASE_FILES) tests/semaphoreStressTest.c $(LINK_FLAGS)

testProducerConsumer: tests/producerConsumerTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testProducerConsumer $(BASE_INCLUDES) $(BASE_FILES) tests/producerConsumerTest.c $(LINK_FLAGS)

testBarrier: tests/barrierTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testBarrier $(BASE_INCLUDES) $(BASE_FILES) tests/barrierTest.c $(LINK_FLAGS)

testMqueue: tests/mqueueTest.c $(BASE_FILES)
	$(CC) $(CFLAGS) -o build/testMqueue $(BASE_INCLUDES) $(BASE_FILES) tests/mqueueTest.c $(LINK_FLAGS)
