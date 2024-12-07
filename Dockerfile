# Build stage
FROM ubuntu:20.04 as builder

# Install dependencies
RUN apt-get update && apt-get --assume-yes install make gcc libsqlite3-dev libncursesw5-dev locales

# Update locales
RUN locale-gen en_US.UTF-8 && update-locale LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8

# Set environmental variables
ENV TERM xterm
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Copy and install codo
WORKDIR /codo
COPY . .
RUN make && make install

ENTRYPOINT  ["codo"]
