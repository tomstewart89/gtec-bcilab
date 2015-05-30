
# Creates some toy test data

# note that for openvibe .csv you need to manually add the freq value as the last item of the two first lines.
# its not done by this script.

nExamples<-30;
nDim<-50;

# Gaussian data
a<-matrix(data=rnorm(nExamples*nDim),nrow=nExamples);
b<-matrix(data=rnorm(nExamples*nDim),nrow=nExamples);

# slightly overlapping classes, dimension 1 is the only one that matters
a[,1]<-a[,1]-2;
b[,1]<-b[,1]+2;

# transform the data a little with a full rank matrix
tol<-0.1;go<-TRUE;
while(go) {
  r<-matrix(runif(nDim*nDim)-0.5,nrow=nDim);
  if(min(svd(r)$d)>tol) {
	go<-FALSE;
  }
}

# add the time column required by openvibe csv reader
aPad<-cbind(matrix(data=seq(1,nExamples),ncol=1),a);
bPad<-cbind(matrix(data=seq(1,nExamples),ncol=1),b);

write.table(aPad,file="class1.csv",row.names=FALSE,sep=",");
write.table(bPad,file="class2.csv",row.names=FALSE,sep=",");

a<-a%*%r;
b<-b%*%r;

aPad<-cbind(matrix(data=seq(1,nExamples),ncol=1),a);
bPad<-cbind(matrix(data=seq(1,nExamples),ncol=1),b);

write.table(aPad,file="class1rot.csv",row.names=FALSE,sep=",");
write.table(bPad,file="class2rot.csv",row.names=FALSE,sep=",");

