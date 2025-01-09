# Build stage
FROM debian:stable-slim as builder

# Install dependencies
RUN apt-get update && apt-get --assume-yes --no-install-recommends install make gcc libsqlite3-dev libncursesw5-dev locales

# Update locales
RUN sed -i 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/g' /etc/locale.gen && locale-gen en_US.UTF-8 UTF-8 && update-locale LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8

# Set environmental variables
ENV TERM xterm
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Copy and install codo
WORKDIR /codo
COPY . .
RUN make

# Cleanup
RUN apt-get --assume-yes remove make gcc && apt-get --assume-yes purge make gcc

ENTRYPOINT  ["codo"]
