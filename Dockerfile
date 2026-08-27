# Get the GCC preinstalled image from Docker Hub
FROM gcc:latest AS build

# Copy the current folder which contains C++ source code to the Docker image under /app
COPY . /app

# Specify the working directory
WORKDIR /app


EXPOSE 3500

RUN apt-get update
RUN apt-get install libssl-dev -y

RUN make


# Run the program output from the previous stage
FROM ubuntu:latest
WORKDIR /app
COPY --from=build /app ./
CMD ["./examples/http/bin"]
#CMD ["./tests/bin"]