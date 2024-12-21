o1-patched update to fix GpuMemLatency

**Probably wrong** because threads/local_size/wave_size is hardcoded to 1 in all `latency_test` invokations.

`apt install -y opencl-headers ocl-icd-opencl-dev` if needed.

```bash
make
cd GpuMemLatency
./GpuMemLatency_amd64 -test latency -platform 0 -device 0
```

---

This is a final legacy release of a CnC-Tested and Reviewed build of clamchowder's microbenchmark suite.  All credit goes to clam.  The PowerPC portions are also attributed to Neggles, and the Loongson code is also attributed to cheese.

Going further, this will be placed in a legacy branch and only receive minor fixes and support.  
