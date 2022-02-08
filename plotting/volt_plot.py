import pylab
import csv

times = []
spike_vals = []

# open csv
with open('/media/sf_vm_shared_folder/carlsim_volt_plot/voltages.csv', newline='') as csvfile:
	csvreader = csv.reader(csvfile, delimiter=',', quotechar='|')
	for row in csvreader:
		voltages = row
with open('/media/sf_vm_shared_folder/carlsim_volt_plot/spike_times.csv', newline='') as csvfile:
	csvreader = csv.reader(csvfile, delimiter=',', quotechar='|')
	for row in csvreader:
		spike_times = row

# cast strings to float values
for i in range(0, len(voltages)):
	voltages[i] = float(voltages[i])
for i in range(0, len(spike_times)):
	spike_times[i] = float(spike_times[i])

# list times
for i in range(0, len(voltages)):
	times.append(i)
# create values for spikes
for i in range(0, len(spike_times)):
	spike_vals.append(1)

# plot
pylab.figure(1)
pylab.plot(times, voltages)
#pylab.show()
pylab.figure(2)
#pylab.plot(spike_times, spike_vals)
pylab.ylim((0,2))
pylab.eventplot(spike_times, linelengths = .03)
pylab.show()