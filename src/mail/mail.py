# mail.py
# Use QQ Mail SMTP SSL to send message.

# This is used to send vertification code or other message for server.
# Detail remains to be filled.
# Consider to communicate with other components in Unix Domain Socket.
 
from smtplib import SMTP_SSL

# input_ fields are remained to be filled by the user component.
input_content = ""                          # Runtime Item
input_receiver_email = "15086980586@qq.com" # Runtime Item

email_server_host = "smtp.qq.com"            # Configuration Item
email_server_port = 465                      # Configuration Item
email_server_login_name = "976693458@qq.com" # Configuration Item
email_server_login_code = "kiymfnhpoqeubdia" # Configuration Item


subject = "Centhoo | No reply"               # Configuration Item.
receiver = input_receiver_email
content = input_content
message = "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s\r\n\r\n"\
            %(email_server_login_name, receiver, subject, content)



server = SMTP_SSL(host = email_server_host, port = email_server_port)
server.login(email_server_login_name, email_server_login_code)
server.helo(email_server_login_name)
server.rcpt(receiver)
server.sendmail(email_server_login_name, receiver, message)
server.quit()