import hashlib
import os
import sys


def write_def(names, archive_path, out_def, stale_outputs):
    archive_hash = hashlib.sha256()
    try:
        with open(archive_path, "rb") as archive:
            for chunk in iter(lambda: archive.read(1024 * 1024), b""):
                archive_hash.update(chunk)
    except OSError as ex:
        sys.stderr.write("error: cannot read archive %s: %s\n" % (archive_path, ex))
        return 1

    lines = ["; Archive SHA-256: %s" % archive_hash.hexdigest(), "EXPORTS"] + ["    %s" % name for name in sorted(names)]
    data = "".join("%s\r\n" % line for line in lines).encode("utf-8")

    # This runs on every build, so leave an unchanged .def alone rather than re-stamping its
    # timestamp and forcing a needless re-link.
    if os.path.exists(out_def):
        with open(out_def, "rb") as existing_def:
            if existing_def.read() == data:
                return 0

    # Delete stale outputs before committing the new .def. If a DLL is still loaded and cannot
    # be deleted, leaving the old .def in place ensures the next build retries the deletion.
    for stale in stale_outputs:
        if os.path.exists(stale):
            try:
                os.remove(stale)
            except OSError as ex:
                sys.stderr.write("error: cannot delete stale output %s: %s\n" % (stale, ex))
                return 1

    with open(out_def, "wb") as output_def:
        output_def.write(data)

    return 0
