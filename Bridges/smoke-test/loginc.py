from selenium import webdriver
from selenium.webdriver.common.action_chains import ActionChains
from selenium.common.exceptions import NoSuchElementException
import time
import sys
import subprocess


loginPass=sys.argv[1]

url=r'https://qa-connect-imodelhubwebsite.bentley.com/Project/0b048e3d-9699-4a6c-82b8-68f79824c810'
driver = webdriver.Chrome("C:\\bridge\\chromedriver.exe")
driver.set_page_load_timeout(150)
driver.get(url)
driver.find_element_by_name('EmailAddress').send_keys('Saddam.khattak@bentley.com')
#driver.find_element_by_name('Password').send_keys('Hkbridges124@')
#driver.find_element_by_id('submitLogon').click()
driver.find_element_by_name('Password').click()
driver.find_element_by_name('loginfmt').send_keys('Saddam.khattak@bentley.com')
driver.find_element_by_id('idSIButton9').click()
time.sleep(10)
driver.find_element_by_id('passwordInput').send_keys(loginPass)
driver.find_element_by_id('submitButton').click()
driver.find_element_by_id('idSIButton9').click()
time.sleep(10)
#if driver.find_element_by_class_name('menu-button'):
#file = open('C:\\bridge\\Version.log', 'r') 
#version=file.read()
#version=version.rstrip("\n\r")
imodelList=['MS_','VM_', 'HYB_']
for index,imodel in enumerate(imodelList):
   print imodelList
   '''driver.find_element_by_class_name('create-button').click()
   time.sleep(5)
   driver.find_element_by_class_name('bnt-margin-enabled').click()
   actions = ActionChains(driver)
   actions.send_keys(imodel)
   actions.perform()
   time.sleep(5)
   driver.find_element_by_class_name('confirmation-button').click()
   time.sleep(22)
   driver.find_element_by_class_name('hf-breadcrumb-text ').click()
   time.sleep(5)'''
print 'Out'
driver.quit()
#subprocess.call([r'close.bat'])









