#!/usr/bin/env python
# -*- coding: utf-8 -*-
from calendar import monthrange
import random
import json
import os

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



def generateData(years,months,rootD):
	for i,YY in enumerate(years):
		print('Processing year: {}'.format(YY))
		ind_GG = 0
		ind_YY = 0
		
		general = initDict()
		yearly = initDict()

		for MM in range(months[i]):
			print('\tmonth: {}'.format(MM))
			MM_dir = rootD+"monthly/{0}/".format(YY)
			if not os.path.exists(MM_dir):
				os.makedirs(MM_dir)
			ind_MM = 0
			ppl_MM_in = 0
			ppl_MM_out = 0
			
			monthly = initDict()
			ndays = monthrange(YY,MM+1)[1]

			for DD in range(1,ndays+1):
				DD_dir = rootD+"daily/{0}/{1}/".format(YY,MM)
				if not os.path.exists(DD_dir):
					os.makedirs(DD_dir)
				ind_DD = 0
				ppl_DD_in = 0
				ppl_DD_out = 0
				daily = initDict(lbl="Hora")

				for hh in range(24):
					for mm in [0,30]:
						ppl_mm_in = 0
						ppl_mm_out = 0
						for mm_ in range(30):
							if mm_%2==0:
								continue
							ppl_in = random.randint(0,10)
							ppl_out = random.randint(0,10)
							ppl_mm_in += ppl_in
							ppl_mm_out += ppl_out

							ind_GG = updateDict(general,ind_GG,YY,MM,DD,hh,mm+mm_,ppl_in,ppl_out)
						
						ppl_DD_in += ppl_mm_in
						ppl_DD_out += ppl_mm_out 

						if mm==0:
							ind_DD = updateDict(daily,ind_DD,YY,MM,DD,hh,30,ppl_mm_in,ppl_mm_out)
						else:
							ind_DD = updateDict(daily,ind_DD,YY,MM,DD,hh+1,0,ppl_mm_in,ppl_mm_out)

				json_data = json.dumps(daily, indent=4, sort_keys=True)
				with open(rootD+"daily/{0}/{1:02d}/{2:02d}.json".format(YY,MM+1,DD),"w") as f:
					f.write(json_data)

				ppl_MM_in += ppl_DD_in
				ppl_MM_out += ppl_DD_out

				ind_MM = updateDict(monthly,ind_MM,YY,MM,DD,0,0,ppl_DD_in,ppl_DD_out)

			# TODO: Save monthly
			json_data = json.dumps(monthly, indent=4, sort_keys=True)
			with open(rootD+"monthly/{0}/{1:02d}.json".format(YY,MM+1),"w") as f:
				f.write(json_data)

			ind_YY = updateDict(yearly,ind_YY,YY,MM,1,0,0,ppl_MM_in,ppl_MM_out)

		json_data = json.dumps(yearly, indent=4, sort_keys=True)
		with open(rootD+"yearly/{0}.json".format(YY),"w") as f:
			f.write(json_data)

		#json_data = json.dumps(general, indent=4, sort_keys=True)
		#with open(rootD+"general/records-{0}.json".format(YY),"w") as f:
		#		f.write(json_data)



def initDict(lbl=None):
	if lbl is None:
		lbl = "Fecha"

	dct = {}
	dct["cols"] = []
	dct["cols"].append({})
	dct["cols"][0]["label"] = lbl
	dct["cols"][0]["type"] = "date"
	dct["cols"].append({})
	dct["cols"][1]["label"] = "Entrantes"
	dct["cols"][1]["type"] = "number"
	dct["cols"].append({})
	dct["cols"][2]["label"] = "Salientes"
	dct["cols"][2]["type"] = "number"
	dct["rows"] = []

	return(dct)

def updateDict(dct,ind,YY,MM,DD,hh,mm,ppl_in,ppl_out):
	dct["rows"].append({})
	dct["rows"][ind]["c"] = []
	dct["rows"][ind]["c"].append({})
	dct["rows"][ind]["c"][0]["v"] = "Date({0},{1},{2},{3},{4},0)".format(YY,MM,DD,hh,mm)
	#dct["rows"][ind]["c"][0]["f"] =  date2string(hh,mm)
	dct["rows"][ind]["c"].append({})
	dct["rows"][ind]["c"][1]["v"] = ppl_in
	dct["rows"][ind]["c"].append({})
	dct["rows"][ind]["c"][2]["v"] = ppl_out

	return(ind+1)


if __name__=='__main__':
	years =[2018,2019,2020]
	months = [12,12,12]
	rootD = './'

	generateData(years,months,rootD)