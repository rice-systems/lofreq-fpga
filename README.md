## Overview

This repo is for research artifacts of paper "[**An FPGA Accelerator for Genome Variant Calling"**](https://ieeexplore.ieee.org/document/9786183) published at [FCCM 2022](https://www.fccm.org/past/2022/technical-program-2022/).

The paper presents an FPGA-based accelerator of [LoFreq](https://github.com/CSB5/lofreq) which runs 30X faster than the multiprocess CPU baseline. The figure below shows the speedup on 7 SARS-CoV-2 datasets. The speedup mainly results from multiple levels of parallelization and pipelining.

<img src="https://user-images.githubusercontent.com/19209239/192879723-d8a65ef7-d026-452c-9fda-48a9c737d25a.png" width="700" height="270" />

The dataflow of LoFreq's key algorithm ([Possion Binomial Distribution](https://github.com/stan-dev/math/blob/504107d0a12cf04507cc4d3a23bf58ae151d191f/stan/math/prim/fun/poisson_binomial_log_probs.hpp)) is shown in the figure below. The number
of Outer Loop Iterations is commonly several million. (More details can be found [here](https://dl.acm.org/doi/pdf/10.1145/3595297).) Straightforward parallelization is prevented due to the data dependency. Our FPGA accelerator achieves massive speedup from exploiting multiple levels of parallelism:
- Processing multiple data in parallel
- Parallelizing memory access, logarithm pre-compute, and inner loop iterations
- Parallelizing and pipelining inner loop iterations

Moreover, a design space exploration is carried to decide the parallelization factor at each level in order to achieve the best performance given finite resources. The figure above shows results for four points in this design space, among which **Design 3** is the best.

<img src="https://github.com/rice-systems/lofreq-fpga/assets/19209239/2c066435-de6f-4f18-a5b8-e5fb085ff38c" width="700" height="300" />

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
## How to Build
1. Make sure [Xilinx Vitis](https://www.xilinx.com/developer/products/vitis.html) is installed.
2. Change configurations in *Makefile* or *src/u250-krnl.cfg* if necessary.
3. Run ```make xclbin```. For Vitis 2021 or prior versions, consider running ```make xclbin``` using faketime with a [timestamp earlier than 20220101](https://support.xilinx.com/s/question/0D52E00006uxPZBSA2/vitis20201-and-vitis20211-an-error-bad-lexical-cast-has-occurred-suddenly).

---
## Citation

Please consider citing our paper if you find the paper/code useful. :)

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
