import subprocess

def create_container(
    login, 
    public_key_path, 
    ssh_port,
    container_dir,
    base_image,
):
    user_dir = container_dir / login
    user_dir.mkdir(exist_ok=True)

    container_name = f"lama-{login}"
    subprocess.run(
        ["docker", "rm", "-f", container_name], 
        stdout=subprocess.DEVNULL, 
        stderr=subprocess.DEVNULL,
    )

    subprocess.run([
        "docker", "run", "-d",
        "--name", container_name,
        "-v", f"{user_dir}:/workspace",
        "-p", f"{ssh_port}:22",
        base_image
    ], check=True)

    subprocess.run([
        "docker", "exec", container_name, "mkdir", "-p", "/root/.ssh"
    ], check=True)
    subprocess.run([
        "docker", 
        "cp", 
        str(public_key_path), 
        f"{container_name}:/root/.ssh/authorized_keys"
    ], check=True)
    subprocess.run([
        "docker", 
        "exec", 
        container_name, 
        "chmod", 
        "600", 
        "/root/.ssh/authorized_keys"
    ], check=True)

    print(f"✅ Created container: {container_name} (SSH port {ssh_port})")

    return container_name