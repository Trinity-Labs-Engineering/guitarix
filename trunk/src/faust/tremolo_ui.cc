b.openHorizontalhideBox("");
b.create_master_slider(PARAM("bpm"), _("BPM"));
b.closeBox();
b.openVerticalBox("");
{
    b.openHorizontalBox("");
    {
	b.insertSpacer();
	b.create_selector_no_caption(PARAM("SINE"));
	b.insertSpacer();
	b.insertSpacer();
	b.openHorizontalTableBox("");
	{
	    b.create_small_rackknobr(PARAM("bpm"), _("BPM"));
	    b.insertSpacer();
	    b.create_small_rackknobr(PARAM("depth"), _("Depth"));
	    b.insertSpacer();
	    b.create_small_rackknob(PARAM("wet_dry"), _("dry/wet"));
	}
	b.closeBox();
    }
    b.closeBox();
}
b.closeBox();
