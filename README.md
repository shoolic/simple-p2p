# Simple P2P

Project for the Internet Technologies classes at the Faculty of Electronics and Information Technologies, Warsaw University of Technology.

The aim of this project was to invent and implement a simple peer-to-peer protocol for exchanging files in a local network.

## Authors

[Przemysław Stawczyk](https://github.com/przestaw)

[Maciej Szulik](https://github.com/shoolic)

[Kamil Zacharczuk](https://github.com/KamZet)

[Wiktor Michalski](https://github.com/wmichalski)

## Requirements

You should have installed:

- `cmake`
- `Boost`
- `Intel TBB`

## Compile program

Run `./create_configs.sh` first.

- If you would like to compile the Release version, run: `cd build/Release; make`

- If you would like to compile the Debug version, run: `cd build/Debug; make`

## Run program

- If you would like to run the Release version:
`./bin/Release/Simple_P2P --my_ip 'IP address to bind' --broadcast_ip 'your local network broadcast address'`

- If you would like to run the Debug version:
`./bin/Debug/Simple_P2P --my_ip 'IP address to bind' --broadcast_ip 'your local network broadcast address'`

## Usage

Type in `help` inside the running program to see available commands.

## Testing guide

### Requirements

You should have installed:

- `docker 18.09+`
- `docker-compose 1.24+`

### Run containers

Run **`N`** instances of the program to simulate a network:

`docker-compose -p simple_p2p up -d --build --scale host=N`

### Enable user input from one of running hosts

Run

`docker attach simple_p2p_host_N`

where **`N`** is a host id.

### Access to container

To access to the container (not to the program user input) to create or read files run:

`docker exec -it simple_p2p_host_N bash`

where **`N`** is a host id.

### Show container logs

Run

`docker logs simple_p2p_host_N`

where **`N`** is a host id.

### Stop testing

Run

`docker-compose -p simple_p2p down`

### Debug mode

If you would like to debug the program in your IDE, go to `docker-debug` directory and run [default command](#run-containers) to run containers.
Configure your IDE to attach to one of the containers and run the program with debugging tools.
