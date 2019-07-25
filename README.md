- 人正常进出
- 人快速进出
- 人慢速进出
- 两个人进入(重叠)
- 一个人在门口闲逛
- 一个人进去又出来

- 两个人在门口交谈


如果两个管之间存在交叉
AS,BS,AE,BE 一个人进去

AS,BS,AE,AS,BE,AE 一个进去后，到达B区未出B区又出来

AS,BS,AE,AS,BE,BS,AE,BE 两个人进去存在交叉

两个人门口闲逛
AS,AE, BS,BE  AS,BS,AE,BE .....波形千千万

一个人只触发A区，
AS,AE AS,AE

均可通过简单的波形匹配进行区分

####特殊情况
1. 人进入到一半再出来
2. 人进入到一半再出来，然后没有出去彻底又进去
3. 人进去出来，只触发A区，然后再进去 (AS,AE,----,AS,BS,AE,BE onePeople) (1AS,1AE, 2AS,2BS,2AE,3AS,2BE,3BS,3AE,3BE)
4. 人进去出来，只触发A区，然后另外一个人从里面出来 (AS,AE,----,BS,AS,BE,AE onePeople)