# 作业6

在作业5中, 我们需要遍历所有的三角形mesh找到光线与其的交点, 如果模型中三角形数量较多, 那么速度则比较慢. 本次作业则使用BVH进行加速.

# Object
注意, 这里我们的Triangle继承自Object, 同时我们加载的一个模型也继承自Object. 只要它们提供对应的接口即可(这样即可保证bounding box之间的寓意连续).

# BVH

首先, BVH的构建先于渲染. 构建过程类似segment tree. query的时间复杂度为log(N).

## BVH构建

参考segment tree即可.