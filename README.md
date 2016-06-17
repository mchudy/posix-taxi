# posix-taxi
An online multiplayer taxi driver game written in C with aim to achieve maximum compliance with POSIX.

### Usage

Build the server program using `make` and run `./taxi_server [port_number]`. You can use `telnet` as client.

### Rules

Your taxi is marked as `#`, taxis of other connected players are marked as `T`. Your goal is to fulfil orders (by picking up passengers at `A` points and dropping them off at `B` points). To turn right send the `r` char and to turn left send the `l` char to the server. Each completed order gives you $20 and each collision with other taxi results in a loss of $50.

### License
MIT License
