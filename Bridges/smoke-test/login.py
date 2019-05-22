#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
from selenium import webdriver
from selenium.webdriver.common.action_chains import ActionChains
from selenium.common.exceptions import NoSuchElementException
import time
import sys



loginPass=sys.argv[1]
version=sys.argv[2]

url=r'https://qa-connect-imodelhubwebsite.bentley.com/Project/0b048e3d-9699-4a6c-82b8-68f79824c810'
driver = webdriver.Chrome("C:\\bridge\\chromedriver.exe")
driver.set_page_load_timeout(150)
driver.get(url)
time.sleep(20)
driver.find_element_by_name('EmailAddress').send_keys('Saddam.khattak@bentley.com')
driver.find_element_by_name('Password').click()
driver.find_element_by_name('loginfmt').send_keys('Saddam.khattak@bentley.com')
driver.find_element_by_id('idSIButton9').click()
time.sleep(10)
driver.find_element_by_id('passwordInput').send_keys(loginPass)
driver.find_element_by_id('submitButton').click()
driver.find_element_by_id('idSIButton9').click()
time.sleep(10)
imodelList=['MS_'+version,'VM_'+version , 'HYB_'+version, 'IFC_'+version ,'BB24_'+version,'FNT_'+version, 'PRN_'+version, 'OFF_'+version, '2D_'+version, 'LVL_'+version, 'SAV_'+version, '3SM_'+version,'CS_'+version,'Multi_'+version]
for index,imodel in enumerate(imodelList):
   time.sleep(10)
   driver.find_element_by_class_name('create-button').click()
   time.sleep(10)
   driver.find_element_by_class_name('bnt-margin-enabled').click()
   actions = ActionChains(driver)
   actions.send_keys(imodel)
   actions.perform()
   time.sleep(10)
   driver.find_element_by_class_name('confirmation-button').click()
   time.sleep(22)
   driver.find_element_by_class_name('hf-breadcrumb-text ').click()
   time.sleep(10)

driver.quit()









