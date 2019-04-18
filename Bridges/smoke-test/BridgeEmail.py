#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import Encoders
import smtplib
import sys
import os

file = open('C:\\bridge\\Version.log', 'r') 
version=file.read()
build = version
#output_path = sys.argv[2]
#fileComp_path = sys.argv[3]

sender = 'Saddam.Khattak@bentley.com'
body= 'iModel Bridge Service - Microstation : '+ build + 'Error Result'
msg = MIMEMultipart('alternative')

cwd1 = os.getcwd()
fileObj = open(r"EmailSubscribers.txt",'r')
receivers = []
for line in fileObj.read().split('\n'):
    receivers.insert(len(receivers),line)

msg['Subject'] = 'Error in the Bridge '+build+'Build'
msg['From'] = sender
msg['To'] = ', '.join(receivers)
content = MIMEText(body, 'html')
msg.attach(content)

part = MIMEBase('application', "octet-stream")
part.set_payload(open('C:\\bridge\\Errors.txt', "rb").read())
Encoders.encode_base64(part)
part.add_header('Content-Disposition', 'attachment', filename="Errors.txt")
msg.attach(part)

'''if fileComp_path != 'null':
    part = MIMEBase('application', "octet-stream")
    part.set_payload(open(fileComp_path+"\FolderComparison_out_baseline.html", "rb").read())
    Encoders.encode_base64(part)
    part.add_header('Content-Disposition', 'attachment', filename="FolderComparison_out_baseline.html")
    msg.attach(part)'''

try:
    smtpObj = smtplib.SMTP('smtp.bentley.com')
    smtpObj.sendmail(sender, receivers,  msg.as_string())         
    print "Successfully sent email"
except smtplib.SMTPException:
    print "Error: unable to send email"
