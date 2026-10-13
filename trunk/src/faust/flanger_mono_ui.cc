b.openHorizontalhideBox("");
b.create_master_slider(PARAM("depth"), _("depth"));
b.closeBox();
b.openVerticalBox("");
{
    b.openHorizontalBox("");
    {
	b.create_small_rackknobr(PARAM("depth"), _("depth"));
	b.create_small_rackknob(PARAM("lfobpm"), _("LFO freq (bpm)"));
	b.create_small_rackknob(PARAM("wet_dry"), _("dry/wet"));
    }
    b.closeBox();
}
b.closeBox();
