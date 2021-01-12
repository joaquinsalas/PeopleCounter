#!/usr/bin/env python
# -*- coding: utf-8 -*-
from calendar import monthrange
import random
import json

def date2string(HH,MM):
	h = str(HH)
	m = str(MM)

	if len(h)==1:
		h = "0"+h
	if len(m)==1:
		m = "0"+m
	if HH<12:
		time = "am"
	else:
		time = "pm"
	
	out = h+":"+m+" "+time

	return(out)


years = [2018,2019,2020]
for YY in years:
	for m in range(12):
		MM = m+1
		ndays = monthrange(YY,MM)[1]
		for d in range(ndays):
			data = {}
			data["cols"] = []
			data["cols"].append({})
			data["cols"][0]["label"] = "Fecha"
			data["cols"][0]["type"] = "date"
			data["cols"].append({})
			data["cols"][1]["label"] = "Entrantes"
			data["cols"][1]["type"] = "number"
			data["cols"].append({})
			data["cols"][2]["label"] = "Salientes"
			data["cols"][2]["type"] = "number"
			data["rows"] = []

			k = 0
			DD = d+1
			random.seed(YY*MM*DD)
			for hh in range(24):
				for mm in [0,30]:
					data["rows"].append({})
					data["rows"][k]["c"] = []
					data["rows"][k]["c"].append({})
					data["rows"][k]["c"][0]["v"] = 'Date({0},{1},{2},{3},{4})'.format(YY,MM,DD,hh,mm)
					#data["rows"][k]["c"][0]["f"] =  date2string(hh,mm)
					data["rows"][k]["c"].append({})
					data["rows"][k]["c"][1]["v"] = random.randint(0,100)
					data["rows"][k]["c"].append({})
					data["rows"][k]["c"][2]["v"] = random.randint(0,100)
					#print('Date({0},{1},{2},{3},{4})'.format(2018,6,dd,i,j))
					k+= 1

			json_data = json.dumps(data, indent=4, sort_keys=True)
#with open("records.json","w") as f:
	#with open("yearly/{0}.json".format(YY),"w") as f:
		#with open("monthly/{0}/{1:02d}.json".format(YY,MM),"w") as f:
			with open("daily/{0}/{1:02d}/{2:02d}.json".format(YY,MM,DD),"w") as f:
				f.write(json_data)






"""
for dd in range(1,):
	for i in range(24):
		for j in [0,30]:
			data["rows"].append({})
			data["rows"][k]["c"] = []
			data["rows"][k]["c"].append({})
			data["rows"][k]["c"][0]["v"] = 'Date({0},{1},{2},{3},{4})'.format(2018,6,dd,i,j)
			data["rows"][k]["c"][0]["f"] =  date2string(i,j)
			data["rows"][k]["c"].append({})
			data["rows"][k]["c"][1]["v"] = random.randint(0,100)
			data["rows"][k]["c"].append({})
			data["rows"][k]["c"][2]["v"] = random.randint(0,100)
			print('Date({0},{1},{2},{3},{4})'.format(2018,6,dd,i,j))
			k+= 1




json_data = json.dumps(data, indent=4, sort_keys=True)

#print(json_data)

with open("records.json","w") as f:
	f.write(json_data)
"""