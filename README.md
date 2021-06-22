# SLIP
A SLIP (RFC 1055 Standard) component.

该 SLIP 组件的核心文件是 slip.c 和 slip.h 这两个文件，依赖第三方 ringbuffer 库。

- 3rd-party 目录下存储依赖的第三方库
- tests 目录下存储单元测试文件

## 如何使用？

SLIP 组件的接口定义在 slip.h 文件里。

首先需要根据实际的硬件、架构等配置一个发送接收接口：

```C
struct slip_config {
    /* Send data to uart. */
    void (*send)(uint8_t *buffer, uint16_t length);

    /**
     * @brief Receive data from uart.
     * 
     * @return int
     * @retval >=0   Receive data length.
     * @return -1    Error.
    */
    int (*recv)(uint8_t *buffer, uint16_t length);
};
```

然后调用下述函数初始化 SLIP 句柄，

```C
int slip_init(struct slip *handler, struct slip_config *config);
```

之后就可以调用 `slip_send_frame()` 函数发送 slip 数据帧，最终的发送接口是配置的 `send()` 函数；调用 `slip_receive_frame()` 函数接收 slip 数据帧，该函数只有在收到一帧数据时才会返回。具体使用可以参考测试代码。

## 测试

若想要运行测试文件，需要先安装 CUnit 单元测试框架，Ubuntu 环境可以参考[CUnit 安装](https://www.jianshu.com/p/250e31aa7280)，然后在 SLIP 目录依次输入下述命令编译链接运行：

```shell
cmake -H. -Bbuild
cmake --build build
./build/slip
```

## 文档

SLIP 解码器状态机可参考 docs 目录下的文档。