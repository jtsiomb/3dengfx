doc_fname := 3dengfx-doc.ps.gz
htmldoc_fname := html/index.html


.PHONY: doc
doc: $(doc_fname)

.PHONY: htmldoc
htmldoc: $(htmldoc_fname)

$(doc_fname):
	cd latex; latex 3dengfx-doc.tex && latex 3dengfx-doc.tex \
		&& dvi2ps -c 3dengfx-doc.ps 3dengfx-doc.dvi && gzip 3dengfx-doc.ps && mv $(doc_fname) ..

$(htmldoc_fname):
	latex2html latex/3dengfx-doc.tex
	mv latex/3dengfx-doc html

.PHONY: clean
clean:
	rm -f $(doc_fname) latex/*.aux latex/*.log latex/*.dvi latex/*.toc
	rm -rf html
