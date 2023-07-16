## Overview

This repo includes the research artifact for our paper [**"An FPGA Accelerator for Genome Variant Calling"**](https://ieeexplore.ieee.org/document/9786183) published at FCCM 2022. 

The paper presents an FPGA-based accelerator enabling [LoFreq](https://github.com/CSB5/lofreq) to call variants 20X faster than the multiprocess CPU baseline. The figure below shows the speedup on 7 SARS-CoV-2 datasets. The speedup mainly results from multiple levels of parallelization and pipelining.

<img src="https://user-images.githubusercontent.com/19209239/192879723-d8a65ef7-d026-452c-9fda-48a9c737d25a.png" width="700" height="270" />

With the same architecture, there are multiple design points with different hardware configurations. Among them, **Design 3** outperforms the rest. The HLS implementation of Design 3 is shared in this repo. It is also easy to change the source code into other design points, which can be done by simply changing the pragmas (one bonus point of using HLS).

---

## Source Directory
```
├── Makefile: commands to build .xo and .xclbin.
├── README.md
└── src 
    ├── Makefile: commands to run Vitis HLS sythesis.
    ├── hls.tcl: tcl script to set up and run HLS C sythesis.
    ├── krnl.c: HLS C source code.
    ├── krnl.h: header file.
    ├── runPre.tcl: building configuration.
    └── u250-krnl.cfg: Vivado implementation strategy control and SLR connectivity specification.
```

---
## Citation

Please consider citing if you find the paper and/or the source code useful. :) 

```
@INPROCEEDINGS{9786183,
  author={Xu, Tiancheng and Rixner, Scott and Cox, Alan L.},
  booktitle={2022 IEEE 30th Annual International Symposium on Field-Programmable Custom Computing Machines (FCCM)}, 
  title={An FPGA Accelerator for Genome Variant Calling}, 
  year={2022},
  volume={},
  number={},
  pages={1-9},
  doi={10.1109/FCCM53951.2022.9786183}}
```
