Windows caches PAC scripts, so changing them in test sources will still use old scripts. Restart does not help. To fix:

Run PACCachingDisable.reg
Restart PC
Now PAC caching is disabled and you can run tests with changes in PAC scripts. Note that PC IE HTTP performance might decrease.

Run PACCachingEnable.reg
Restart
Now PAC caching is enabled again for whole PC and new PAC scripts work.