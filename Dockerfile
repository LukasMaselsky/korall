# Get the GCC preinstalled image from Docker Hub
FROM gcc:latest AS build

# Copy the current folder which contains C++ source code to the Docker image under /opt/test
COPY . /opt/test

# Specify the working directory
WORKDIR /opt/test

EXPOSE 3500

RUN apt-get update
RUN apt-get install libssl-dev -y

RUN make


# Run the program output from the previous stage
FROM ubuntu:latest
WORKDIR /opt/test
COPY --from=build /opt/test ./
CMD ["./examples/http/bin"]