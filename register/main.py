import csv
from pathlib import Path

from gen_password import gen_password
from gen_ssh import gen_ssh_keys
from container_creator import create_container
from email_sender import send_access_email

BASE_IMAGE = "lama-rv-workshop-lama-rv:latest"
ACCESS_FILE = "access.csv"
CONTAINERS_DIR = Path("./containers")
CSV_FILE = "participants.csv"
SSH_PORT_BASE = 2200
CONTAINERS_DIR.mkdir(exist_ok=True)

def main():
    with open(CSV_FILE) as f_in, open(ACCESS_FILE, "w") as f_out:
        reader = csv.DictReader(f_in)
        writer = csv.writer(f_out)
        writer.writerow(
            [
                "email", 
                "login", 
                "password", 
                "container_name", 
                "ssh_port", 
                "private_key",
            ]
        )

        for i, row in enumerate(reader):
            login = row["login"].strip()
            email = row["email"].strip()
            password = gen_password()
            ssh_port = SSH_PORT_BASE + i

            private_key, public_key = gen_ssh_keys(login, CONTAINERS_DIR)
            container_name = create_container(
                login, 
                public_key, 
                ssh_port,
                CONTAINERS_DIR,
                BASE_IMAGE,
            )

            writer.writerow([
                email, 
                login, 
                password, 
                container_name, 
                ssh_port, 
                str(private_key),
            ])
            send_access_email(email, login, password, str(private_key))
    print("\n✅ All containers are ready.")
    print(f"Access info saved to {ACCESS_FILE}")

if __name__ == "__main__":
    main()