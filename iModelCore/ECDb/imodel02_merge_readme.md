# How to merge bim0200dev to the ECDb folder in the imodel02 repo

Merging bim0200dev to the imodel02 repo cannot be done automatically anymore as the two repos diverged. Merging
therefore has to be done manually.

## Use tag **mergetag_imodel02** in manual merges

The tag **mergetag_imodel02** in the ECDb repo on bim0200dev can come to help for manual merges. It indicates that all changes up to and including the tagged changeset were already handled by previous merges. So only changesets newer than the tag have to be
considered in merges.

## Move tag **mergetag_imodel02** after manual merge

That also means, that you must move the tag **mergetag_imodel02** after having done a new merge.