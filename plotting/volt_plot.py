import pylab
import csv

times = []

# open csv
with open('/media/sf_vm_shared_folder/nest/voltages.csv', newline='') as csvfile:
	csvreader = csv.reader(csvfile, delimiter=',', quotechar='|')
	for row in csvreader:
		voltages = row

# cast strings to float values
for i in range(0, len(voltages)):
	voltages[i] = float(voltages[i])

# list times
for i in range(0, len(voltages)):
	times.append(i)

# plot
pylab.figure(1)
pylab.plot(times, voltages)
pylab.show()