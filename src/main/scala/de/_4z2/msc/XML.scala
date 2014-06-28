package de._4z2.msc

import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.Map
import scala.io.Source
import scala.xml._
import scala.xml.pull._

object XmlParser {
	def processNode(node: scala.xml.Node, id: Int, tree: OrderedTree[TreeNode, TreeEdge], map: Map[Int, String]) {
		val children = node \ "_"
		children.foreach(child => {
			val childId = tree.addNode
			tree.addEdge(id, childId)
			assert(tree.nodes(childId).parent == id)
			map(childId) = child.label
			if (childId % 10000 == 0) {
				println(tree.summary)
				tree.gc()
			}
			processNode(child, childId, tree, map)
		})
	}

	def parse(filename: String) = {
		val root = XML.loadFile(filename)
		val tree = new OrderedTree[TreeNode, TreeEdge](() => new TreeNode(), () => new TreeEdge())
		val map = Map[Int, String]()
		val id = tree.addNode
		map(id) = root.label
		processNode(root, id, tree, map)
		tree.gc()
		(tree, map)
	}
}


object LazyXmlParser {
	def parse(filename: String) = {
		val xml = new XMLEventReader(Source.fromFile(filename))
		val tree = new OrderedTree[TreeNode, TreeEdge](() => new TreeNode(), () => new TreeEdge())
		val map = ArrayBuffer[String]()

		def processNode(parents: List[Int]) {
			if (xml.hasNext) {
				xml.next match {
					case EvElemStart(_, label, _, _) =>
						//println("Start element: " + label + " parents: " + parents)
						val nodeId = tree.addNode
						assert(map.size == nodeId)
						map += label
						if (parents.size > 0) {  // if this is not the root, add an edge to the parent
							tree.addEdge(parents.head, nodeId)
						}
						if (nodeId % 10000 == 0) {
							println(tree.summary)
							tree.gc()
						}
						processNode(nodeId :: parents)
					case EvElemEnd(_, label) =>
						//println("End element: " + label)
						processNode(parents.tail)
					case _ => processNode(parents)
				}
			}
		}
		processNode(List.empty)
		tree.gc()
		(tree, map)
	}
}