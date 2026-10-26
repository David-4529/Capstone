# Midterm submittal: running list of updates

Collected while reviewing the filled Word document. Apply everything at the end in one pass.

**Current working version: `Wind_Project_-_Midterm_Final_Submittal_2.docx`**, your edited document with Section 2.2 and the single-line diagram added. Older versions (`_CONDENSED`, `_FILLED`) are kept for reference.

Status key: [ ] open, [x] done

## Make it match what the assignment asks for

- [ ] **2.1 Wind Prospector:** add a Wind Prospector screenshot of Sites A/B/C (100 m wind speed, land use and transmission layers). My composite map was removed in the condensed version.
- [x] **2.2.1 FAA:** run the FAA OE/AAA Notice Criteria Tool for each site (lat/long, ground elevation 34 / 5 / 1 ft, 492 ft structure height). Paste the results and update the text to match.
- [x] **2.2.2 Land cover:** the template asks for NLCD or PAD-US. The current percentages come from ESA WorldCover. Replace them with NLCD values (or a PAD-US Data Explorer check) and add screenshots.
- [x] **2.2.3 USFWS:** run IPaC for each project area and replace the expected species list with the official IPaC list.
- [x] **2.2.4 NCNHP:** run the NCNHP Data Explorer (site plus a 1-mile buffer) and replace the text with the report results.
- [x] **Section 3 data source:** Lecture 1 (slide 29) lists two accepted methods, the API or direct HDF5 (HSDS) access. The report data came from the direct HDF5 method, so update the Section 3 wording to cite it. Optional: run `NLR_wind_data_download_SiteA.m` (API method, add your key) to re-download the same data. Its quick check should reproduce 6.52 / 6.91 m/s, α = 0.262, k = 2.40, a = 7.69. **Done:** Section 3 wording now cites the Lecture 1 HSDS/HDF5 method.
- [ ] **4.4 / 5.5 / 6.4 SAM (graduate):** the runs used SAM's engine (PySAM), not the SAM desktop application. Repeat them in SAM desktop (Wind + Single Owner) and add screenshots of the inputs and results.
- [x] **7 Permitting:** cut to one page in the condensed version.
- [ ] **7 Interconnection:** the template references Duke Energy procedures, but the site is in Dominion/PJM territory. Decide whether to keep the Dominion/PJM explanation or reframe around Duke.

## Citations and captions

- [ ] **All figures made from data** (condensed version: 3-1, 3-2, 3-3, 4-1, 8-2): add data sources to the captions. Do not say "prepared by the authors" unless you rebuild the figures yourselves.
- [ ] **4.1:** cite the current Vestas V110-2.0 MW datasheet (URL and date). The power curve is from the SAM library curve "Vestas V110-2.0".
- [ ] **6.1 grid price:** check the latest EIA NC industrial price (Electric Power Monthly Table 5.6.A), or switch to a Duke Energy or Dominion industrial rate.
- [ ] **7 / Ref [12]:** look up and cite the current Perquimans County wind ordinance (setbacks, height, noise, shadow flicker).
- [ ] **References:** reformat to the citation style the instructor requires.
- [ ] **3.6 (optional):** cite an extreme-wind source (ASCE 7 / NOAA) against the 37.5 m/s Class III limit.

## Final checks

- [ ] Remove all yellow highlighted notes once each item is done.
- [ ] Open the document in Word and confirm the equations render. They show blank in LibreOffice/PDF previews only.
- [ ] Check the cover date (currently 10/09/26).
- [ ] Rework the text in your own words and make sure both team members can explain every number.

## Notes added during review

- [x] Condensed the whole document to template length, and removed figures I had added (composite map, wind rose, monthly, diurnal-power and duration charts) plus extra sensitivity text.
- [ ] **8.1 Site plan:** open `SiteA_layout.kml` in Google Earth (turbines, collection lines, project substation, POI, gen-tie), take a screenshot, and insert it as Figure 8-1.
- [ ] **Table 2-1:** the land, transmission, wetland and setback rows are preliminary values from OpenStreetMap/ESA WorldCover. Replace them with your Wind Prospector, NLCD and FAA values.

- [x] **5.1:** bin table rebuilt as a compact table (1–10 and 11–20 m/s side by side), and all tables now stay on the same page as their captions.

- [ ] Statistical Review of World Energy and bp Energy Outlook 2025 (Lecture 1 readings): not required by any template section. Only cite them if the instructor wants background context.

- [x] **2.2 intro + 2.2.1–2.2.4:** rewritten from your FAA, PAD-US, IPaC and NCNHP results. Caption typo "MCNHP" changed to "NCNHP".
- [x] **8.2 SLD:** Figure 8-2 added (4 feeders × 4 turbines to match your site plan, 5.8 km gen-tie) with a description paragraph. Section 8.1 text changed from 2.8 km to 5.8 km.
- [ ] **2.2.1 distances:** the new text uses the Table 2-1 distances (KEDE 9.5 mi, USCG 7.3 mi, Hyde Co. 3.5 mi) and the 28 mi radar zone. Your earlier text said ~10 / ~10 / ~5 mi and a 30 mi zone. Keep Table 2-1 and the text consistent.
- [ ] **IPaC table (2-2-3-1):** check it against the IPaC output. For Site C, red knot, eastern black rail and red-cockaded woodpecker appear in both the Endangered and Threatened rows. The monarch is listed Endangered for Site C but Threatened for A/B (IPaC shows it as proposed threatened). The tricolored bat is "proposed endangered".
- [ ] **Table 2-1 Wetlands row:** Sites B and C say "Feasible, but land is fragmented" / "Feasible", which looks copied from the setbacks row. Should read something like "Extensive (marsh, open water)" / "Extensive (herbaceous wetland)".
- [ ] **Duplicate caption:** both the screening table and the yearly wind speed table are captioned "Table 2-1". Renumber one (e.g., Table 2-2).
- [ ] **Section 2.1 figure captions:** the screenshots appear to be from the NLR WRDB (Wind Resource Database) viewer. If so, name it that way in the captions rather than "WIND Prospector".
- [ ] **Section 4.3 / Figure 4-1:** they still describe 5 rows (2/7/3/1/3), but your site plan uses 4 rows of 4. Update the text and redraw Figure 4-1 to match. Your rows also run east–west, while 4.3 says rows are perpendicular to the 210° wind; either rotate the rows or adjust the wording.

<!-- new items go here as you go through the document -->
