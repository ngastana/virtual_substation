auth_url = "https://cloud.tecnalia.dev:13000/v3"
region   = "regionOne"

user_name        = "cooperacion-111306@tecnalia.com"
user_domain_name = "tri.lan"
password         = "1Sondika!!!"

project_name      = "virtualizacióndenodoslógicos"
project_domain_id = "3e6dcdec8ad44563a8334675306bc571"

instance_name = "rhel-docker-host"
image_name    = "ubuntu-22.04-server-cloudimg-amd64"
flavor_name   = "std2_4"
#flavor_name     = "gpu2_2"
network_name        = "nw_int_virtualizacióndenodoslógicos"
keypair_name        = "ssh-key"
public_key_path     = "~/.ssh/id_ed25519.pub"
security_groups     = ["default", "sg_virtualizacióndenodoslógicos"] #openstack security group show sg_virtualizacióndenodoslógicos
insecure            = true
docker_volume_name  = "docker-volume"
docker_volume_size  = 50
private_key_path    = "~/.ssh/id_ed25519"
images_tar_path     = "substation_images.tar"
docker_compose_file = "docker-compose.yml"
#cacert_file     = "/home/nerea/Downloads/ca.pem"
external_network_id = "b58d8f92-c96c-43fb-a591-2dcd8e3d04a2" #IP de nw_lab_bizkaia_trc red publica openstack --insecure network list
#external_network_id = "62d08de1-83e7-4086-944a-685eaa93f35e"  #IP de nw_int_virtualizacióndenodoslógicos red privada openstack --insecure network list