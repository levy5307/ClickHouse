# ClickHouse Server Docker Image

## What is ClickHouse?

ClickHouse is an open-source column-oriented database management system that allows generating analytical data reports in real time.

ClickHouse manages extremely large volumes of data in a stable and sustainable manner. It currently powers [Yandex.Metrica](https://metrica.yandex.com/), world’s [second largest](http://w3techs.com/technologies/overview/traffic_analysis/all) web analytics platform, with over 13 trillion database records and over 20 billion events a day, generating customized reports on-the-fly, directly from non-aggregated data. This system was successfully implemented at [CERN’s LHCb experiment](https://www.yandex.com/company/press_center/press_releases/2012/2012-04-10/) to store and process metadata on 10bn events with over 1000 attributes per event registered in 2011.

For more information and documentation see https://clickhouse.com/.

## How to use this image

### start server instance
```bash
$ docker run -d --name some-clickhouse-server --ulimit nofile=262144:262144 clickhouse/clickhouse-server
```

By default ClickHouse will be accessible only via docker network. See the [networking section below](#networking).

By default, starting above server instance will be run as default user without password.

### connect to it from a native client
```bash
$ docker run -it --rm --link some-clickhouse-server:clickhouse-server --entrypoint clickhouse-client clickhouse/clickhouse-server --host clickhouse-server
# OR
$ docker exec -it some-clickhouse-server clickhouse-client
```

More information about [ClickHouse client](https://clickhouse.com/docs/en/interfaces/cli/).

### connect to it using curl

```bash
echo "SELECT 'Hello, ClickHouse!'" | docker run -i --rm --link some-clickhouse-server:clickhouse-server curlimages/curl 'http://clickhouse-server:8123/?query=' -s --data-binary @-
```
More information about [ClickHouse HTTP Interface](https://clickhouse.com/docs/en/interfaces/http/).

### stopping / removing the containter

```bash
$ docker stop some-clickhouse-server
$ docker rm some-clickhouse-server
```

### networking

You can expose you ClickHouse running in docker by [mapping particular port](https://docs.docker.com/config/containers/container-networking/) from inside container to a host ports:

```bash
$ docker run -d -p 18123:8123 -p19000:9000 --name some-clickhouse-server --ulimit nofile=262144:262144 clickhouse/clickhouse-server
$ echo 'SELECT version()' | curl 'http://localhost:18123/' --data-binary @-
20.12.3.3
```

or by allowing container to use [host ports directly](https://docs.docker.com/network/host/) using `--network=host` (also allows archiving better network performance):

```bash
$ docker run -d --network=host --name some-clickhouse-server --ulimit nofile=262144:262144 clickhouse/clickhouse-server
$ echo 'SELECT version()' | curl 'http://localhost:8123/' --data-binary @-
20.12.3.3
```

### Volumes

Typically you may want to mount the following folders inside your container to archieve persistency:

* `/var/lib/clickhouse/` - main folder where ClickHouse stores the data
* `/val/log/clickhouse-server/` - logs

```bash
$ docker run -d \
	-v $(realpath ./ch_data):/var/lib/clickhouse/ \
	-v $(realpath ./ch_logs):/var/log/clickhouse-server/ \
	--name some-clickhouse-server --ulimit nofile=262144:262144 clickhouse/clickhouse-server
```

You may also want to mount:

* `/etc/clickhouse-server/config.d/*.xml` - files with server configuration adjustmenets
* `/etc/clickhouse-server/usert.d/*.xml` - files with use settings adjustmenets
* `/docker-entrypoint-initdb.d/` - folder with database initialization scripts (see below).

### Linux capabilities

ClickHouse has some advanced functionality which requite enabling several [linux capabilities](https://man7.org/linux/man-pages/man7/capabilities.7.html).

It is optional and can be enabled using the following [docker command line agruments](https://docs.docker.com/engine/reference/run/#runtime-privilege-and-linux-capabilities):

```bash
$ docker run -d \
	--cap-add=SYS_NICE --cap-add=NET_ADMIN --cap-add=IPC_LOCK \
	--name some-clickhouse-server --ulimit nofile=262144:262144 clickhouse/clickhouse-server
```

## Configuration

Container exposes 8123 port for [HTTP interface](https://clickhouse.com/docs/en/interfaces/http_interface/) and 9000 port for [native client](https://clickhouse.com/docs/en/interfaces/tcp/).

ClickHouse configuration represented with a file "config.xml" ([documentation](https://clickhouse.com/docs/en/operations/configuration_files/))

### Start server instance with custom configuration
```bash
$ docker run -d --name some-clickhouse-server --ulimit nofile=262144:262144 -v /path/to/your/config.xml:/etc/clickhouse-server/config.xml clickhouse/clickhouse-server
```

### Start server as custom user
```
# $(pwd)/data/clickhouse should exist and be owned by current user
$ docker run --rm --user ${UID}:${GID} --name some-clickhouse-server --ulimit nofile=262144:262144 -v "$(pwd)/logs/clickhouse:/var/log/clickhouse-server" -v "$(pwd)/data/clickhouse:/var/lib/clickhouse" clickhouse/clickhouse-server
```
When you use the image with mounting local directories inside you probably would like to not mess your directory tree with files owner and permissions. Then you could use `--user` argument. In this case, you should mount every necessary directory (`/var/lib/clickhouse` and `/var/log/clickhouse-server`) inside the container. Otherwise, image will complain and not start.

### Start server from root (useful in case of userns enabled)
```
$ docker run --rm -e CLICKHOUSE_UID=0 -e CLICKHOUSE_GID=0 --name clickhouse-server-userns -v "$(pwd)/logs/clickhouse:/var/log/clickhouse-server" -v "$(pwd)/data/clickhouse:/var/lib/clickhouse" clickhouse/clickhouse-server
```

### How to create default database and user on starting

Sometimes you may want to create user (user named `default` is used by default) and database on image starting. You can do it using environment variables `CLICKHOUSE_DB`, `CLICKHOUSE_USER`, `CLICKHOUSE_DEFAULT_ACCESS_MANAGEMENT` and `CLICKHOUSE_PASSWORD`:

```
$ docker run --rm -e CLICKHOUSE_DB=my_database -e CLICKHOUSE_USER=username -e CLICKHOUSE_DEFAULT_ACCESS_MANAGEMENT=1 -e CLICKHOUSE_PASSWORD=password -p 9000:9000/tcp clickhouse/clickhouse-server
```

## How to extend this image

If you would like to do additional initialization in an image derived from this one, add one or more `*.sql`, `*.sql.gz`, or `*.sh` scripts under `/docker-entrypoint-initdb.d`. After the entrypoint calls `initdb` it will run any `*.sql` files, run any executable `*.sh` scripts, and source any non-executable `*.sh` scripts found in that directory to do further initialization before starting the service.
Also you can provide environment variables `CLICKHOUSE_USER` & `CLICKHOUSE_PASSWORD` that will be used for clickhouse-client during initialization.

For example, to add an additional user and database, add the following to `/docker-entrypoint-initdb.d/init-db.sh`:

```bash
#!/bin/bash
set -e

clickhouse client -n <<-EOSQL
	CREATE DATABASE docker;
	CREATE TABLE docker.docker (x Int32) ENGINE = Log;
EOSQL
```

## License

View [license information](https://github.com/ClickHouse/ClickHouse/blob/master/LICENSE) for the software contained in this image.
