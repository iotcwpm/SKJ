#!/usr/bin/Rscript

# Script to render all stencils and export as HTML

require(stencila)

for(address in c(
	'model/description',
	'parameters/description',
	'model/display',
	'data/display'
)){
	Stencil(address)$render()$export(paste0(address,'/index.html'))
}
