import smtplib
from email.message import EmailMessage

SMTP_SERVER = "smtp.gmail.com"
SMTP_PORT = 587
SMTP_USER = "playboicartina@gmail.com"
SMTP_PASSWORD = ""

def send_access_email(email, login, password, private_key_str):
    msg = EmailMessage()
    msg["Subject"] = "Your Workshop Access"
    msg["From"] = SMTP_USER
    msg["To"] = email

    msg.set_content(f"""
Hello {login},

Here are your access details for the workshop:

Login: {login}
Password: {password}

The private key is attached to this email.

Best regards,
Workshop Team
""")

    msg.add_attachment(private_key_str.encode(), 
                       maintype="application", 
                       subtype="octet-stream", 
                       filename=f"{login}_id_rsa")

    with smtplib.SMTP(SMTP_SERVER, SMTP_PORT) as server:
        server.starttls()
        server.login(SMTP_USER, SMTP_PASSWORD)
        server.send_message(msg)
        print(f"✅ Email sent to {email}")