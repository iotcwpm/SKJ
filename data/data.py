'''
Python script for various data processing tasks
'''

import urllib
import sh

def get_files():
	'''
	Obtain data files for skipjack as listed at http://www.iotc.org/English/meetings/wp/wpttcurrent.php
	'''
	
	# Make data directory if it does not yet exist
	sh.mkdir('source-data',p=True)

	# Retreive various files
	base_url = 'http://www.iotc.org/files/proceedings/2013/wptt/'
	for filename in (
		'CAT_TROP13.pdf', # Data catalogue

		'NCTROP.zip', # Catch data sources and quality information
		'CEALL.zip', # All catch and effort data zipped up
		'CE_Reference.zip', # Information on catch and effort files

		'FL_SKJ.zip',
		'SF_Reference.zip',
	):
		urllib.urlretrieve(base_url+filename,'source-data/'+filename)

if __name__=='__main__':
	get_files()