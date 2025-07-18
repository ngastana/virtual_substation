# terraform {
#   required_providers {
#     openstack = {
#       source  = "terraform-provider-openstack/openstack"
#       version = "~> 3.0"
#     }
#   }
# }

# provider "openstack" {
#   auth_url          = var.auth_url
#   user_domain_name  = var.user_domain_name
#   project_domain_id = var.project_domain_id
#   tenant_name       = var.project_name
#   user_name         = var.user_name
#   password          = var.password
#   region            = var.region

#   insecure = var.insecure
#   #  cacert_file = var.cacert_file
# }

# resource "openstack_compute_keypair_v2" "ssh_key" {
#   name       = var.keypair_name
#   public_key = file(var.public_key_path)
# }

# resource "openstack_compute_instance_v2" "rhel" {
#   name            = var.instance_name
#   image_name      = var.image_name
#   flavor_name     = var.flavor_name
#   key_pair        = openstack_compute_keypair_v2.ssh_key.name
#   security_groups = var.security_groups

#   network {
#     name = var.network_name
#   }
# }




#El anterior código crea una máquina virtual RHEL en OpenStack, 
#instala Docker y Docker Compose, y carga imágenes de contenedores 
#desde un archivo tar. Ahora monta un volumen para almacenar los 
#datos de Docker.

terraform {
  required_providers {
    openstack = {
      source  = "terraform-provider-openstack/openstack"
      version = "~> 3.0"
    }
  }
}

provider "openstack" {
  auth_url          = var.auth_url
  user_domain_name  = var.user_domain_name
  project_domain_id = var.project_domain_id
  tenant_name       = var.project_name
  user_name         = var.user_name
  password          = var.password
  region            = var.region
  insecure          = var.insecure
}

data "openstack_networking_network_v2" "public_net" {
  name = "nw_lab_bizkaia_trc"
}

resource "openstack_networking_network_v2" "private_net" {
  name = "nw_int_virtualizacióndenodoslógicos"
  admin_state_up = true
}

resource "openstack_networking_subnet_v2" "private_subnet" {
  name       = "custom-subnet-with-dns"
  network_id = openstack_networking_network_v2.private_net.id
  cidr       = "192.168.77.0/24"
  ip_version = 4
  gateway_ip = "192.168.77.1"
}

resource "openstack_networking_router_v2" "router" {
  name                = "router-to-ext"
  admin_state_up      = true
  external_network_id = data.openstack_networking_network_v2.public_net.id
}

resource "openstack_networking_router_interface_v2" "router_interface" {
  router_id = openstack_networking_router_v2.router.id
  subnet_id = openstack_networking_subnet_v2.private_subnet.id
}

resource "openstack_networking_secgroup_v2" "ssh" {
  name = "allow_ssh"
}

resource "openstack_networking_secgroup_rule_v2" "ssh_rule" {
  direction         = "ingress"
  ethertype         = "IPv4"
  protocol          = "tcp"
  port_range_min    = 22
  port_range_max    = 22
  remote_ip_prefix  = "0.0.0.0/0"
  security_group_id = openstack_networking_secgroup_v2.ssh.id
}

resource "openstack_compute_keypair_v2" "ssh_key" {
  name       = var.keypair_name
  public_key = file(var.public_key_path)
}

resource "openstack_networking_port_v2" "vm_port" {
  name       = "ied-host-port"
  network_id = openstack_networking_network_v2.private_net.id
  fixed_ip {
    subnet_id = openstack_networking_subnet_v2.private_subnet.id
  }
  security_group_ids = [openstack_networking_secgroup_v2.ssh.id]
}

resource "openstack_networking_floatingip_v2" "fip" {
  pool = data.openstack_networking_network_v2.public_net.name
}

resource "openstack_networking_floatingip_associate_v2" "fip_assoc" {
  floating_ip = openstack_networking_floatingip_v2.fip.address
  port_id     = openstack_networking_port_v2.vm_port.id
  depends_on = [
    openstack_networking_router_interface_v2.router_interface
  ]
}

locals {
  ieds = [
    for dir in fileset("${path.module}", "ied*/docker-compose.yml") :
    dirname(dir)
  ]
}

resource "openstack_blockstorage_volume_v3" "ied_data" {
  for_each = toset(local.ieds)
  name     = "volume-${each.key}"
  size     = var.docker_volume_size
}

#lanzamos la VM
resource "openstack_compute_instance_v2" "ied_host" {
  name            = "ied-host"
  image_name      = var.image_name
  flavor_name     = var.flavor_name
  key_pair        = openstack_compute_keypair_v2.ssh_key.name
  security_groups = var.security_groups

  network {
    port = openstack_networking_port_v2.vm_port.id
  }
}

#cada volumen (de cada IED) se conecta a la máquina virtual
resource "openstack_compute_volume_attach_v2" "attach_ied_data" {
  for_each    = toset(local.ieds)
  instance_id = openstack_compute_instance_v2.ied_host.id
  volume_id   = openstack_blockstorage_volume_v3.ied_data[each.key].id
}

resource "null_resource" "copy_ied_data" {
  for_each = toset(local.ieds)

  connection {
    type        = "ssh"
    host        = openstack_networking_floatingip_v2.fip.address
    user        = "ubuntu"
    private_key = file(var.private_key_path)
  }

  provisioner "file" {
    source      = "${path.module}/${each.key}" # Local path: e.g., ied1/
    destination = "/home/ubuntu/${each.key}"   # Remote path
  }

  depends_on = [
    openstack_compute_volume_attach_v2.attach_ied_data
  ]
}

resource "null_resource" "provision_all_ieds" {
  depends_on = [
    openstack_compute_volume_attach_v2.attach_ied_data,
    openstack_networking_floatingip_associate_v2.fip_assoc
  ]

  triggers = {
    always_run = timestamp()
  }

  connection {
    type        = "ssh"
    host        = openstack_networking_floatingip_v2.fip.address
    user        = "ubuntu"
    private_key = file(var.private_key_path)
  }

  provisioner "remote-exec" {
    inline = [
      #"set -xe",

      "echo ' Configurando DNS y resolv.conf...'",
      "sudo systemctl stop systemd-resolved || true",
      "sudo systemctl disable systemd-resolved || true",
      "sudo rm -f /etc/resolv.conf",
      "echo 'nameserver 172.26.100.102\nnameserver 172.26.100.103\nnameserver 172.17.100.102\nnameserver 172.17.100.103\nnameserver 172.19.100.101' | sudo tee /etc/resolv.conf",
      "echo '127.0.0.1 rhel-docker-host' | sudo tee -a /etc/hosts",

      "echo '🧰 Montando volumen de Docker en /var/lib/docker...'",
      "sudo apt-get update && sudo apt-get install -y ca-certificates curl gnupg lsb-release",
      "sudo mkdir -m 0755 -p /etc/apt/keyrings",
      "curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor --yes -o /etc/apt/keyrings/docker.gpg",
      "echo \"deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable\" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null",
      "sudo apt-get update && sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin",
      "sudo systemctl enable docker && sudo systemctl start docker",
      "docker --version || echo '⚠️ Docker no disponible'",
      "docker compose version || echo '⚠️ docker compose no disponible'",

      "echo ' Montando discos IED en /mnt/ied*...'",
      "count=1",
      "for dev in $(lsblk -ndo NAME,TYPE | awk '$2==\"disk\" {print $1}' | grep -v vda); do",
      "  mount_point=/mnt/ied$count",
      "  echo ' Formateando y montando /dev/$dev en $mount_point'",
      "  sudo mkfs.ext4 -F /dev/$dev || true",
      "  sudo mkdir -p $mount_point",
      "  mountpoint -q $mount_point || sudo mount /dev/$dev $mount_point",
      "  echo \"/dev/$dev $mount_point ext4 defaults 0 2\" | sudo tee -a /etc/fstab",
      "  count=$((count + 1))",
      "done",

      "echo '📦 Copiando datos de IEDs a /mnt/ied*...'",
      "count=1",
      "for ied in /home/ubuntu/ied*/; do",
      "  dst=/mnt/ied$count",
      "  echo \"📁 Copiando desde $ied a $dst\"",
      "  sudo rsync -a --delete \"$ied\"/ \"$dst\"/ || echo '⚠️ Fallo en sincronización, puede estar vacío'",
      "  count=$((count + 1))",
      "done",

      "echo '🐳  Levantando servicios Docker con docker-compose...'",
      "for d in /mnt/ied*; do",
      "  for img in \"$d\"/*.tar; do",
      "    [ -f \"$img\" ] && echo \"Cargando $img\" && sudo docker load -i \"$img\"; done",
      "  [ -f \"$d/docker-compose.yml\" ] && sudo docker compose -f \"$d/docker-compose.yml\" up -d",
      "done"
    ]
  }
}

output "ied_detectados" {
  value = local.ieds
}

output "floating_ip" {
  value = openstack_networking_floatingip_v2.fip.address
}
