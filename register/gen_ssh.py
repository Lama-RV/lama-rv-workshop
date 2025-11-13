import subprocess

def gen_ssh_keys(login, container_dir):
    ssh_dir = container_dir / login / "ssh"
    ssh_dir.mkdir(parents=True, exist_ok=True)
    private_key = ssh_dir / "id_rsa"
    public_key = ssh_dir / "id_rsa.pub"

    subprocess.run([
        "ssh-keygen", "-t", "rsa", "-b", "4096",
        "-f", str(private_key), "-N", "", "-C", login
    ], check=True)
    return private_key, public_key