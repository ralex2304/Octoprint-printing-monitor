"""
Log master library by ralex2304.
Version: 1.0
"""

import datetime

def log(log_text):
	n = datetime.datetime.now()
	timestamp="["+str(n.strftime("%d"))+"/"+str(n.strftime("%m"))+"/"+str(n.strftime("%y"))+" "+str(n.strftime("%H"))+":"+str(n.strftime("%M"))+":"+str(n.strftime("%S"))+"]"
	return timestamp+"- "+log_text