安装 CppUTest：

```shell
ubuntu@VM-16-6-ubuntu:~/tools$ git clone https://github.com/cpputest/cpputest.git
ubuntu@VM-16-6-ubuntu:~/tools$ cd cpputest/
ubuntu@VM-16-6-ubuntu:~/tools/cpputest$ mkdir build-cmake && cd build-cmake/
ubuntu@VM-16-6-ubuntu:~/tools/cpputest/build-cmake$ cmake ..
ubuntu@VM-16-6-ubuntu:~/tools/cpputest/build-cmake$ make -j
ubuntu@VM-16-6-ubuntu:~/tools/cpputest/build-cmake$ sudo make install
```

编译运行测试用例：

```shell
sudo apt install gcovr lcov
cmake -H. -Bbuild -DWITH_COVERAGE=ON
cmake --build build
./build/test_slip
```

测试覆盖率：97.9%。查看测试覆盖率报告：

```shell
cmake --build build -t coverage
google-chrome build/coverage/index.html
```