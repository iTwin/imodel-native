// automatically generated, do not modify

package Example

import (
	flatbuffers "github.com/google/flatbuffers/go"
)
type Vec3 struct {
	_tab flatbuffers.Struct
}

func (rcv *Vec3) Init(buf []byte, i flatbuffers.UOffsetT) {
	rcv._tab.Bytes = buf
	rcv._tab.Pos = i
}

func (rcv *Vec3) X() float32 { return rcv._tab.GetFloat32(rcv._tab.Pos + flatbuffers.UOffsetT(0)) }
func (rcv *Vec3) Y() float32 { return rcv._tab.GetFloat32(rcv._tab.Pos + flatbuffers.UOffsetT(4)) }
func (rcv *Vec3) Z() float32 { return rcv._tab.GetFloat32(rcv._tab.Pos + flatbuffers.UOffsetT(8)) }
func (rcv *Vec3) Test1() float64 { return rcv._tab.GetFloat64(rcv._tab.Pos + flatbuffers.UOffsetT(16)) }
func (rcv *Vec3) Test2() int8 { return rcv._tab.GetInt8(rcv._tab.Pos + flatbuffers.UOffsetT(24)) }
func (rcv *Vec3) Test3(obj *Test) *Test {
	if obj == nil {
		obj = new(Test)
	}
	obj.Init(rcv._tab.Bytes, rcv._tab.Pos + 26)
	return obj
}

func CreateVec3(builder *flatbuffers.Builder, x float32, y float32, z float32, test1 float64, test2 int8, Test_a int16, Test_b int8) flatbuffers.UOffsetT {
    builder.Prep(16, 32)
    builder.Pad(2)
    builder.Prep(2, 4)
    builder.Pad(1)
    builder.PrependInt8(Test_b)
    builder.PrependInt16(Test_a)
    builder.Pad(1)
    builder.PrependInt8(test2)
    builder.PrependFloat64(test1)
    builder.Pad(4)
    builder.PrependFloat32(z)
    builder.PrependFloat32(y)
    builder.PrependFloat32(x)
    return builder.Offset()
}
