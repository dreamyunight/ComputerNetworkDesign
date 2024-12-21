# 《计算机网络》课程设计题目

* ## <u>煤矿安全生产物联网协议设计及软件实现</u>

  在煤矿生产中，需要他用过物联网监测矿井下的环境，并且，设置相应的安全设施保证安全生产。具体业务如下：

  1. 实时监测井下甲烷、瓦斯、氧气、一氧化碳、温度、烟雾、粉尘等传感器和通风开关、电扇、报警蜂鸣器等联动设备，这些数据又分为模拟量（也就是传感器的值是一个具体数值，例如甲烷含量、当前温度等）和开关量（也就是只有两个状态的传感器，例如：烟雾的有，用 1 表示，无，用 0 表示，**<u>设备运行开关量</u>** 1 表示在运行，0 表示已停止，馈电传感器 1 表示有电，0 表示无电。**<u>馈电传感器需要关联到设备运行开关量</u>**），而且，对于保障工人生命健康的通风设备，需要有主要和备用两个组成，主、备不能同时处于相同状态。
  2. 系统结构为井上有一个服务器运行上位机软件（服务器端），井下有一台嵌入式主机运行下位机软件（客户端），嵌入式主机连接多个传感器和控制设备。上位机和下位机通过网络通信。
     <u>**井下下位机正常每五秒上传一次数据，如果甲烷高于上限值，则不需要等待，立即上传；</u>**上位机收到数据后要给出确认，然后再根据传来的数据采取动作。
  3. 设计应用层协议，用于传输井下各传感器数据到服务器，服务器收到井下数据后，需要根据传感器状态做出相应的反应。例如：温度过高就需要通知井下设备打开风扇，瓦斯浓度过高就需要对所控制的设备进行断电，使其停止工作（**<u>设备运行开关量设置为 0 </u>**），**==同时要检测相应的馈电传感器是否已经为无电状态，如果不是则表示断电失败，需要报警（打开蜂鸣器）==**，主备同时为开或者同时为关，就需要在服务器端提示管理员。根据业务需要，设计应用层协议，要体现出协议的三要素，使得协议数据格式尽可能合理高效。
  4. 使用 socket 编程，实现上位机，接收来自客户端的数据。实现一个物联网客户端，通过 TCP 连接到上位机，并用软件模拟传感器和控制设备。客户端产生数据，发送至服务器，收到服务器传感的数据后，在客户端能反映出控制设备的动作。

  >  **备注：每一个服务器会连接多个客户端，需要在服务器端能够识别出数据来自于哪一个客户端。协议要求用字节（二进制）表示，需要对数据进行校验，以检查收到的数据是否在传输过程中出错。**

  要求：设计报告要体现出协议设计的方法（使用 IP 和 TCP 协议首部类似的格式进行描述）；设计成果要能正确运行并反映出系统的状态。

