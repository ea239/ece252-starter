## Lab-2

paster is a small C program that reconstructs a full comic image from 50 separate PNG “strips” stored on three mirror servers.

**How it works**

1. At start-up you can specify

* `-t <threads>` – number of worker threads (default 1)
* `-n <img_id>`  – which image set to fetch (1 – 3; default 1).
  Invalid flags or an `-n` value greater than 3 cause an immediate error message and exit.

2. Each worker thread repeatedly:

* chooses one of the three servers at random,
* downloads a strip with libcurl
* if the strip hasn’t been seen before, saves it as `output/output_<seq>.png`,
* logs: `Got strip <seq>, <remaining> remaining`.

1. A shared mutex protects the `downloaded[]` bitmap and `total_download` counter; file I/O happens outside the lock, keeping contention low.
   Threads reset their private `fail_count` on every successful strip and only quit after 1000 consecutive errors, so at least one thread always keeps running until all 50 pieces arrive.

2. When `total_download` reaches 50 the main thread joins all workers, calls `cat_png()` function to stitch the 50 files vertically into `all.png`, deletes the temporary `output` directory, and exits 0.

**Result**

The program reliably fetches every strip and delivers a final combined PNG, finishing quickly and cleanly even if one of the servers is slow or unresponsive.
