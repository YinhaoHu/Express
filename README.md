# Express: Documentation for Developer( Status: On developing )
---

## Overview

`Express` is a social software designed to facilitate self-expression in various social situations, including public interactions, communication with friends, and engagement within specific interest communities.

## Business Functionality

The software offers the following fundamental functionalities:

- **Chat with Friends**: Users can engage in one-on-one conversations with their friends within the application.

- **Chat in a Friend Group**: Users have the ability to participate in group conversations with their friends.

- **Post in Interest Communities**: Users can create and share posts within specific interest communities, fostering discussions around shared topics of interest.

## Business Components

- **Friend System**: This component manages the relationship between users, enabling friend requests, acceptance, and communication.

- **Community System**: The community system governs the creation, management, and interactions within interest-based communities. 

---

## Function Components

### Daemon

The Daemon is a background process that runs continuously, performing various tasks and services to support the functionality of the application. It takes the responsibilities to initialize the other components according to the configurations.

### Log

The Log component is responsible for recording important events and messages from the application. It facilitates troubleshooting and debugging efforts.

### Database

The DataBase component is used to store the persistent data in the software system.It is a similar table-form database but is not identical with that. It does not have any memory cache or other optimization technology in order to make it easier to implement. Its structure is visualized in `database-design.drawio`.

### Mail Service

> This component will be implemented by Python.

The Mail Service component handles communication and notifications related to emails and messages sent within the application. One of its use is sending veritification code.

### Workers

The Workers are responsible for executing various tasks and computations required for the application's functioning. They are part of the master-workers architecture.

### Monitor

The Monitor component monitors the health and performance of the application in real-time. It keeps track of system metrics and identifies potential issues or bottlenecks.

---

## Architecture

The software is designed using a `microservice architecture`, which allows for the development of loosely coupled and independently deployable services. This architecture enhances scalability, maintainability, and resilience.

Additionally, the software employs the `master-workers` pattern, which distributes tasks among multiple workers managed by a master process. This pattern optimizes performance and resource utilization.

---

## Misccellaneous



### Configure File



This file should include at least the following items. And the format of each configuration item should be like`KEY=VALUE` in every line. `#Comment` should be the comment format. The least configuration items.

- Component host: local or remote

- Component address: IP address or IPC name. (Based on the implementation)

- Component port: IP port or NULL (Based on the implementation)

### 

### Deloyed Automatically



Hassle-free experience of deloyment is what we persue. Express server is designed and should be desiged to be deloyed automatically in most effort and the server administrator can deloy it with hassle-free.
