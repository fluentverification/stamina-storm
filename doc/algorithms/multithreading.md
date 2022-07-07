# Multithreading STAMINA Algorithm

In order to properly create multithreading in STAMINA, we must add mutual exclusions to the state space at some level, however there is a tradeoff in memory impact and CPU time:
	+ If we place a Mutex on every single state, we see a massive memory impact. The `std::mutex` and the C equivalent `pthread_mutex_t` are both 40 bytes.
	+ If we place one mutex on the entire state space then we see no benefit to multithreading, since every state that is modified must first lock the entire state space for all other threads.
Therefore, 

**Exploration Thread Object**

```
OBJECT ExplorationThread
	DATA MEMBERS
		PUBLIC CONST i -> INT // (written on construction)
		PUBLIC numberStatesOwned := INT
		PUBLIC finished -> bool // (many read, one write)
		PUBLIC X -> queue() (mutex)
		PRIVATE S -> queue()
	END DATA MEMBERS

	METHODS
		PUBLIC PROCEDURE doExploration() -> VOID
			WHILE shouldNotDie DO
				// Weak priority on cross exploration
				IF NOT X.empty() AND NOT X.locked() THEN
					s := dequeue(X)
					explore(s)
				ELSE IF NOT S.empty() THEN
					s := dequeue(S)
					explore(s)
				ELSE
					finished := true
				END IF
			END DO
		END PROCEDURE

		PUBLIC PROCEDURE explore(s -> state) -> VOID
			FOR EACH s' in next(s)
				IF owner(s') == null THEN
					o := requestOwnership(s')
					IF o == i THEN
						exploreSuccessor(s')
					END IF
				ELSE IF owner(s') != i THEN
					threads[owner(s')].X.enqueue(s')
				ELSE
					exploreSuccessor(s')
				END IF
			END FOR
		END PROCEDURE

		PUBLIC PROCEDURE exploreSuccessor(s' -> state) -> VOID
			(STAMINA 2.0 block)
			TODO: Add STAMINA 2.0 block
		END PROCEDURE
	END METHODS
END OBJECT
```

**Worker Thread Object**

```
OBJECT WorkerThread
	DATA MEMBERS
		PRIVATE ownedThreads -> HashMap<state, INT>()
		PRIVATE ownershipMutex -> SharedMutex() (Mutex that allows multiple reads but only one write)
		PUBLIC transitionMatrixQueues -> list<LockableQueue<INT, INT, double>>() (One queue for each of the threads to prevent the need for sharing mutual exclusion)
	END DATA MEMBERS

	METHODS
		PUBLIC PROCEDURE requestOwnership(state, threadIndex) -> INT
			IF state in ownedThreads THEN
				RETURN ownedThreads.get(state)
			ELSE
				TRY LOCK ownershipMutex
				WHEN LOCK achieved THEN
					(Checks to see of some other state beat us to the lock)
					IF state IN ownedThreads THEN
						RETURN ownedThreads.get(state)
					ELSE
						CLAIM OWNERSHIP OF STATE
						ownedThreads.insert(state, threadIndex)
						RETURN threadIndex
					END IF
				END WHEN
			END IF
		END PROCEDURE

		PUBLIC PROCEDURE whoOwns(state) -> INT
			(DOES NOT LOCK MUTEX)
			IF state IN ownedThreads THEN
				RETURN ownedThreads.get(state)
			ELSE
				RETURN null
			END IF
		END PROCEDURE

		PUBLIC PROCEDURE mainLoop() -> VOID
			INT numberFinishedThreads := 0
			DO WHILE numberFinishedThreads < numberThreads
				FOR EACH queue IN transitionMatrixQueues DO
					WHILE NOT queue.empty() DO
						fromState, toState, rate := queue.dequeue()
						transitionMatrixEntries.insert(fromState, toState, rate)
					END WHILE
					IF THE CORRESPONDING THREAD IS FINISHED
						numberFinishedThreads := numberFinxploredThreads + 1
						(At some point we should add some defragmentation to transfer thread ownership)
					END IF
				DONE
			END DO
		END PROCEDURE
	END METHODS
END OBJECT
```

**Model Builder Object**

```
OBJECT ModelBuilder
	DATA MEMBERS
		threads -> list()
		transitionMatrixEntries -> TransitionMatrixEntries() (Asynchronous helper for transition matrix creation)
	END DATA MEMBERS
	BEGIN METHODS
		PUBLIC PROCEDURE exploreStates() -> VOID
			initQueue := queue()
			si := initialState()
			numberPerimeterStates := 0
			initQueue.enqueue(si)
			DO WHILE numberPerimeterStates < numberThreads
				s := dequeue(initQueue)

				ENQUEUE ALL successor states and update PI/EXPLOREDMAPS/PERIMETERSTATES
				numberPerimeterStates := numberPerimeterStates - 1
			END WHILE
			index := 1
			FOR EACH s IN PERIMETER STATES DO
				threads[index].X.enqueue(s)
				index := index + 1
			END FOR
			WHILE anyThreadWorking() DO
				IDLE
			END WHILE
		END PROCEDURE
	END METHODS
END OBJECT
```
