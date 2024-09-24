#!/usr/bin/env python
# coding: utf-8

# In[1]:


import numpy as np
import matplotlib as plt
import h5py

import os, sys
from pathlib import Path

master_file = h5py.File('results/rtp-test/master.h5', 'w')


# In[2]:


h5files = filter(lambda x: x.endswith('h5') and x != 'master.h5', os.listdir('results/rtp-test/'))
h5files = list(h5files)
h5files.sort()


# In[3]:


for file in h5files:
    try:
        f = h5py.File(f'results/rtp-test/{file}')
        grp = master_file.create_group(f'{Path(file).stem}')
    
        for key in f['metadata'].keys():
            grp[f'metadata/{key}'] = f[f'metadata/{key}'][()]
    
        for key in f['rawdata'].keys():
            vds_shape = f[f'rawdata/{key}'].shape
            vds_layout = h5py.VirtualLayout(vds_shape, np.float64)
            vds_layout[:] = h5py.VirtualSource(file, name=f'rawdata/{key}', shape=(5000,), dtype=np.float64, maxshape=(None,))
            grp.create_virtual_dataset(f'rawdata/{key}', vds_layout, fillvalue=-1)
    except:
        print(file)


# In[ ]:




