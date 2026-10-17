.PHONY: run debug release clean tests scenarios benchmarks
run: release tests
	./yazdan-backend/build/engine
	./yazdan-backend/build/tests_scenarios
	./yazdan-backend/build/tests_benchmarks

SRC = ./yazdan-backend/src/ardunio.cpp ./yazdan-backend/src/comms.cpp ./yazdan-backend/src/logger.cpp ./yazdan-backend/src/logic.cpp ./yazdan-backend/src/physichs.cpp

debug: clean setup tests
	g++ ./yazdan-backend/src/main.cpp $(SRC) -o ./yazdan-backend/build/engine -O0 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
release: clean setup tests
	g++ ./yazdan-backend/src/main.cpp $(SRC) -o ./yazdan-backend/build/engine -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
setup:
	mkdir -p ./yazdan-backend/build
	mkdir -p ./yazdan-backend/build/obj
	mkdir -p ./yazdan-backend/build/scenarios
	mkdir -p ./pipe
	> ./yazdan-backend/logs.txt
	touch ./pipe/python2cpp.pipe

scenarios: setup
	./yazdan-backend/tests/run_scenarios.py

scenarios-single: setup
	g++ ./yazdan-backend/tests/test_scenarios.cpp $(SRC) -o ./yazdan-backend/build/tests_scenarios -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
	./yazdan-backend/build/tests_scenarios

benchmarks: setup
	g++ ./yazdan-backend/tests/test_benchmarks.cpp $(SRC) -o ./yazdan-backend/build/tests_benchmarks -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
	./yazdan-backend/build/tests_benchmarks

tests: setup
	g++ ./yazdan-backend/tests/test_scenarios.cpp $(SRC) -o ./yazdan-backend/build/tests_scenarios -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
	g++ ./yazdan-backend/tests/test_benchmarks.cpp $(SRC) -o ./yazdan-backend/build/tests_benchmarks -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
	g++ ./yazdan-backend/tests/test_virtual_sim.cpp $(SRC) -o ./yazdan-backend/build/tests_virtual_sim -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20

visualizer: setup
	g++ ./yazdan-backend/tests/test_visualizer.cpp $(SRC) -o ./yazdan-backend/build/visualizer -O3 -g -Wall -Wextra -lraylib -std=c++20
	./yazdan-backend/build/visualizer

clean:
	rm -rf ./yazdan-backend/build/engine ./yazdan-backend/build/tests_* ./yazdan-backend/build/visualizer ./yazdan-backend/build/obj ./yazdan-backend/build/scenarios