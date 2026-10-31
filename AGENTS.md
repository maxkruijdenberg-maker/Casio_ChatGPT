# Publishing customer downloads

When this task includes publishing or repairing downloadable packages:

- Keep ATOM/Rev E V40.1 firmware separate from the Rev G XIAO firmware. The boards have different UART pins/polarity.
- Rebuild the customer ZIP with `python3 tools/package_customer.py` after its inputs change. This verifies the compiled-artifact SHA256 manifest; it does not compile firmware.
- Maintain `downloads/manifest.json`. For a changed payload, use a NEW `download-…-rN` tag. Never force-update or delete an existing download tag to replace its content.
- Use `python3 tools/publish_downloads.py publish` for authorized publication. It creates payload-only tags without changing the checkout/staging area, pushes tags, verifies public access, downloads the actual GitHub archives, checks every file and embedded firmware checksum, then generates DOWNLOADS.md and downloads/verified.json.
- Prefer the ordinary GitHub archive links from the newly verified DOWNLOADS.md. Do not invent release-asset URLs or share an unverified raw URL.
- Do not report publication success when verification fails. A 404, HTML response, private repository, changed content or BOM/CPL mismatch must stop publication confirmation. Do not change repository visibility without explicit authorization.
- Before sharing download links, ensure the verification report matches the manifest and the public ZIPs were checked after pushing. Keep GitHub infrastructure availability distinct from what the verifier proves.
