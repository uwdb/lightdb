# About LightDB

LightDB is a database management system (DBMS) designed
to efficiently ingest, store, and deliver virtual reality (VR)
content at scale. LightDB currently targets both live and prerecorded
spherical panoramic (a.k.a. 360°) and light field VR videos. It persists content
as a multidimensional field that includes both spatiotemporal 
and angular (i.e., orientation) dimensions. 
ontent delivered through LightDB offers improved throughput,
less bandwidth, and scales to many concurrent connections.

See our [website](http://lightdb.uwdb.io) for more details.

### Installing LightDB

1. Docker

Install a containerized version of LightDB via DockerHub

```sh
docker pull lightdb/core
docker run -it lightdb/core
```

2. Build from source

To build from source, clone the repository, install dependencies, generate a Makefile, and build:

```sh
git clone https://github.com/uwdb/lightdb.git
cat packages.txt | xargs sudo apt-get install
mkdir build
cd build
cmake ..
make
```

### Using LightDB

LightDB support declarative queries over _temporal light fields_ (TLF).  To obtain a reference to a TLF, `scan` it from the internal LightDB catalog or `load` it from disk:

```python
    tlf = Scan("internal-tlf")
    tlf2 = Load("~/video.mp4")
```

Each of the algebraic operators described in section 3.2 of the [LightDB paper](http://db.cs.washington.edu/projects/lightdb/p1144-haynes.pdf) may be applied to a TLF.  For example, to apply the predictive 360° query described in [section 3.5](http://db.cs.washington.edu/projects/lightdb/p1144-haynes.pdf), a user would write the following query:

```python
    query = Scan("internal-tlf")
              .Partition(Dimension::Theta, degrees{90})
              .Partition(Dimension::Phi, degrees{90})
              .Subquery(lambda tlf: tlf.Encode(Codec.hevc, bitrate=b))
              .Store("result")
    Coordinator().execute(query)
```

Note that this query performs the workload presented in our SIGMOD'16 VideoCloud demonstration and discussed in the video shown below.  Note that LightDB's architecture and functionality now extend far beyond this use case; see the paper for more details.

### Questions?

Open an [issue](https://github.com/uwdb/lightdb/issues) with any questions or issues you might have.

## Citations & Paper

If you use LightDB, please cite our VLDB'18 paper:

_LightDB: A DBMS for Virtual Reality Video_<br/>
Brandon Haynes, Amrita Mazumdar, Armin Alaghi, Magdalena Balazinska, Luis Ceze, and Alvin Cheung<br />
VLDB:1192-1205 [[PDF]](http://www.vldb.org/pvldb/vol11/p1192-haynes.pdf)

```
@article{DBLP:journals/pvldb/HaynesMABCC18,
  author    = {Brandon Haynes and
               Amrita Mazumdar and
               Armin Alaghi and
               Magdalena Balazinska and
               Luis Ceze and
               Alvin Cheung},
  title     = {{LightDB}: {A} {DBMS} for Virtual Reality Video},
  journal   = {{PVLDB}},
  volume    = {11},
  number    = {10},
  pages     = {1192--1205},
  year      = {2018},
  doi       = {10.14778/3231751.3231768},
}
```

#### Acknowledgments

This work is supported by the National Science Foundation
through NSF grants 
[CCF-1703051](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1703051), 
[IIS-1247469](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1247469), 
[IIS-1546083](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1546083), 
[CCF-1518703](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1518703), and 
[CNS-1563788](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1563788);
DARPA award [FA8750-16-2-0032](https://www.darpa.mil); DOE award [DE-SC0016260](https://science.energy.gov/grants);
a [Google Faculty Research Award](https://docs.google.com/document/d/1IfCmWZ-ClmvmB4gzlApR4htAhYBjKliPGQxLpu6KmaU/edit);
an award from the [University of Washington Reality Lab](https://realitylab.uw.edu);
and gifts from the [Intel Science and Technology Center for Big
Data](http://istc-bigdata.org), [Intel Corporation](https://www.intel.com), [Adobe](http://www.adobe.com), [Amazon](https://www.amazon.com), [Facebook](https://facebook.com), [Huawei](https://www.huawei.com) and [Google](https://google.com).

&nbsp;
