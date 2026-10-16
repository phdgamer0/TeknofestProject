.PHONY: run debug release clean tests
run: release tests
	./yazdan-backend/build/engine
	./yazdan-backend/build/tests_*_*
debug: clean setup tests
	g++ ./yazdan-backend/src/main.cpp -o ./yazdan-backend/build/engine -O0 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
release: clean setup tests
	g++ ./yazdan-backend/src/main.cpp -o ./yazdan-backend/build/engine -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
setup:
	mkdir -p ./yazdan-backend/build
	mkdir -p ./pipe
	touch ./yazdan-backend/logs.txt
	touch ./pipe/python2cpp.pipe
tests:
	g++ ./yazdan-backend/tests/test_*_*.cpp -o ./yazdan-backend/build/tests_*_* -O3 -g -Wall -Wextra -Wpedantic -Werror -flto -std=c++20
clean:
	rm -f ./yazdan-backend/build/engine ./yazdan-backend/build/tests_*_*