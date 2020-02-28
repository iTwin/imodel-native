// automatically generated, do not modify

package MyGame.Example;

import java.nio.*;
import java.lang.*;
import java.util.*;
import com.google.flatbuffers.*;

public class Stat extends Table {
  public static Stat getRootAsStat(ByteBuffer _bb) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (new Stat()).__init(_bb.getInt(_bb.position()) + _bb.position(), _bb); }
  public Stat __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public String id() { int o = __offset(4); return o != 0 ? __string(o + bb_pos) : null; }
  public ByteBuffer idAsByteBuffer() { return __vector_as_bytebuffer(4, 1); }
  public long val() { int o = __offset(6); return o != 0 ? bb.getLong(o + bb_pos) : 0; }

  public static int createStat(FlatBufferBuilder builder,
      int id,
      long val) {
    builder.startObject(2);
    Stat.addVal(builder, val);
    Stat.addId(builder, id);
    return Stat.endStat(builder);
  }

  public static void startStat(FlatBufferBuilder builder) { builder.startObject(2); }
  public static void addId(FlatBufferBuilder builder, int idOffset) { builder.addOffset(0, idOffset, 0); }
  public static void addVal(FlatBufferBuilder builder, long val) { builder.addLong(1, val, 0); }
  public static int endStat(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
};

