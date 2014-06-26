package de._4z2.msc

import scala.collection.mutable.ArrayBuffer

class Cluster(val left: Int, val right: Int, val mergeType: Int) {
	var rLeft: Int = -1
	var lRight: Int = -1
	var rRight: Int = -1
	var height: Int = -1
	var size: Int = -1
	var distTBleft: Int = -1
	var distTBright: Int = -1

	override def toString: String = "(" + left + "," + right + "/" + mergeType + ")"
}

class TopTree(val numLeaves: Int) {
	val clusters = new ArrayBuffer[Cluster](numLeaves)
	clusters ++= (0 until numLeaves).map(i => new Cluster(-1, -1, -1))

	def addCluster(left: Int, right: Int, mergeType: Int) = {
		clusters += new Cluster(left, right, mergeType)
		clusters.size - 1
	}

	override def toString: String = "Top tree: " + clusters
}