all: index doxygen

.PHONY: index doxygen publish

index:
	stencila --convert index.cila index.html

doxygen:
	$(MAKE) -C doxygen
	
# Publish documentation
# Aggregates alternative documentation into a single place for publishing using Github pages 
# Requires the a branch called "gh-pages":
#	git branch gh-pages
# and the "ghp-import" script
# 	sudo pip install ghp-import
publish:
	mkdir -p published
	cp -f index.html published
	cp -fr doxygen published
	ghp-import -m "Updated documentation" -p published

clean:
	rm -f index.html
	$(MAKE) -C doxygen clean
