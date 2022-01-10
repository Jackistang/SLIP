# SLIP
SLIP (RFC 1055 Standard) 组件。

基本的编解码功能已完成，并且通过了单元测试，详见 tests 目录。

需求分析，测试设计，接口设计详见 images/slip.xmind 文件。

TODO：

1. 解码器目前无法连续解码。

-----

解码器状态图：

![decoder_state_machine](images/decoder_state_machine.png)

