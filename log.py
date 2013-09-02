# -*- coding=utf-8 -*-
import logging

class log:
	def __init__(self):
		log = logging.getLogger()
		if len(logging.root.handlers) == 0:
			#ch = logging.StreamHandler()
			ch = logging.FileHandler('logfile')
			formatter = logging.Formatter('%(levelname)s - %(asctime)s - %(message)s')
			ch.setFormatter(formatter)
			log.addHandler(ch)
			log.setLevel(logging.NOTSET)
		self.logger = log

	def shutdown(self):
		#self.logger.shutdown()
		logging.shutdown()
	
	def debug(self, str):
		self.logger.debug(str)

	def info(self, str):
		self.logger.info(str)

	def warn(self, str):
		self.logger.warn(str)

	def error(self, str):
		self.logger.error(str)

	def exception(self, str):
		self.logger.error('Exception: %s' % str)
	
	def Exception(self, str):
		self.logger.error('Exception: %s' % str)
	



