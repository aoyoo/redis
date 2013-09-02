#!/usr/bin/env python
# -*- coding=utf-8 -*-
import os, time, sys
import redis
import log
from exception import *

port = 6389
rdb_file = './db_bak/dump_db_sec.rdb'
bak_path = './bak'

logger = log.log()

def connect_redis():
	try:
		r = redis.StrictRedis(port=port)
		if r.ping():
			logger.info('connect success')
			return r
		else:
			logger.Exception('ping error')
			raise ping_error
	except Exception, data:
		logger.Exception(data)
		raise connect_error
		
def dump_rdb(r):
	rdb_bgsave_st = r.info().get('rdb_bgsave_in_progress') 
	# 1 mean is on bgsave, 0 mean not
	if rdb_bgsave_st == None:
		#what happend!!!
		logger.Exception('get rdb_bgsave_in_progress None')
		raise dump_rdb_error
	elif rdb_bgsave_st == 0:
		try:
			r.bgsave()
		except redis.exceptions.ResponseError, data:
			error_data = '%s' % data
			if error_data == 'Background save already in progress': 
				#block until bgsave success, unlidely
				while True:
					tmp_rdb_bgsave_st = r.info().get('rdb_bgsave_in_progress') 
					if tmp_rdb_bgsave_st == 0:
						break;
					logger.error('unlidely rdb still on bgsave')
					time.sleep(5)
		except Exception, data:
			logger.Exception(data)
			raise dump_rdb_error
	
		while True:
			tmp_rdb_bgsave_st = r.info().get('rdb_bgsave_in_progress') 
			if tmp_rdb_bgsave_st == 0:
				break;
			logger.info('rdb still on bgsave')
			time.sleep(5)
		logger.info('rdb bgsave success')

	elif rdb_bgsave_st == 1:
		while True:
			tmp_rdb_bgsave_st = r.info().get('rdb_bgsave_in_progress') 
			if tmp_rdb_bgsave_st == 0:
				break;
			logger.info('rdb still on bgsave')
			time.sleep(5)
	else:
		logger.error('get rdb_bgsave_in_progress %s' % rdb_bgsave_st)
		raise dump_rdb_error

def bak_rdb_file():
	local_time = time.strftime('%Y-%m-%d',time.localtime(time.time()))
	bak_file_name = '%s/%s_%s.rdb' % (bak_path, local_time, port)
	copy_cmd = 'cp %s %s' % (rdb_file, bak_file_name)
	os.popen(copy_cmd)
	logger.info('rdb file bak success %s' % bak_file_name)
	delete_old_rdb = """
					find %s -maxdepth 1 -type d -mtime +20 -exec rm -fr {} \;
					""" % bak_path
	os.popen(delete_old_rdb)
	
if __name__  ==  "__main__":
	try:
		r = connect_redis()
		dump_rdb(r)
		bak_rdb_file()
	except Exception, data:
		print 'Exception sys.exit(1) %s' % data
		sys.exit(1)
		
	
	


