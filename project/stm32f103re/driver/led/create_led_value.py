# -*- coding: UTF-8 -*-
import numpy
import matplotlib.pyplot as plt
import math

max_value = 500
point_number = 400


#  6/point_number != 6.0/point_number
step = 6.0/point_number
print('step=', step)

plt.figure(1)

# set plot shap
ax = plt.subplot(111)

tmp_lx = list(numpy.arange(-3,3,step))
lx = []
ly = []
for tmp_x in tmp_lx:
    tmp_y1 = 1 / (math.sqrt(2 * numpy.pi))
    tmp_y2 = - (tmp_x * tmp_x) / 2
    tmp_y3 =tmp_y1 *  math.exp(tmp_y2)
    ly.append(int(tmp_y3 * 2.5 * max_value))
    lx.append(int((tmp_x + 3) * point_number / 6))

print(ly)
ax.plot(lx, ly)
plt.show()
