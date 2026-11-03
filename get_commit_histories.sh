echo "Mason Simmons" >> commits.txt;
git log --author=f >> commits.txt;
echo "Niko Krinos" >> commits.txt;
git log --author=j >> commits.txt;
echo "Austin Works" >> commits.txt;
git log --author=a >> commits.txt;

sed "/^Author/d" < commits.txt > tempcommits.txt;
sed "/^commit/d" < tempcommits.txt > commits.txt;
sed "s/Date:   / - /g" < commits.txt > finalcommits.txt;
